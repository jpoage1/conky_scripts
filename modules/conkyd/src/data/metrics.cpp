// metrics.cpp
#include "metrics.hpp"

#include "cpuinfo.hpp"
#include "data.hpp"
#include "data_local.hpp"
#include "data_ssh.hpp"
#include "diskstat.hpp"
#include "filesystems.hpp"
#include "load_avg.hpp"
#include "polling.hpp"
#include "processinfo.hpp"
#include "provider.hpp"
#include "runner.hpp"
#include "ssh.hpp"
#include "sysinfo.hpp"
#include "uptime.hpp"
void SystemMetrics::complete() {}
SystemMetrics::SystemMetrics(MetricsContext& context) {
  switch (context.provider) {
    case DataStreamProviders::LocalDataStream: {
      std::unique_ptr<DataStreamProvider> local_provider =
          std::make_unique<LocalDataStreams>();
      provider = std::move(local_provider);
    } break;
    case DataStreamProviders::ProcDataStream: {
      // Check for specific host/user
      if (context.host != "" && context.user != "") {
        // ProcDataStreams ssh_provider(context.host, context.user);
        //   provider = std::move(ssh_provider);
      } else {
        std::unique_ptr<DataStreamProvider> ssh_provider =
            std::make_unique<ProcDataStreams>();
        provider = std::move(ssh_provider);
      }
    } break;
  }
  load_device_paths(context.device_file);
}

bool SystemMetrics::load_device_paths(const std::string& config_file) {
  // Clear any existing paths
  this->device_paths.clear();

  namespace fs = std::filesystem;

  // Check if the file exists, is a regular file, and is readable
  if (!fs::exists(config_file) || !fs::is_regular_file(config_file)) {
    std::cerr << "Unable to load device path file: " + config_file << std::endl;
    return false;  // Return false on failure
  }

  std::ifstream file(config_file);
  if (!file.is_open()) {
    std::cerr << "Unable to open device path file: " + config_file << std::endl;
    return false;
  }

  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty()) {
      // Add directly to the member variable
      this->device_paths.push_back(line);
    }
  }
  return true;  // Return true on success
}

int SystemMetrics::read_data() {
  CpuPollingTaskPtr cpu_tasks =
      std::make_unique<CpuPollingTask>(*provider, *this);
  polling_tasks.push_back(std::move(cpu_tasks));

  NetworkPollingTaskPtr network_polling =
      std::make_unique<NetworkPollingTask>(*provider, *this);
  polling_tasks.push_back(std::move(network_polling));

  DiskPollingTaskPtr disk_polling =
      std::make_unique<DiskPollingTask>(*provider, *this);
  polling_tasks.push_back(std::move(disk_polling));

  cpu_temp_c = provider->get_cpu_temperature();

  get_mem_usage(provider->get_meminfo_stream(), meminfo, swapinfo);

  uptime = get_uptime(provider->get_uptime_stream());
  cpu_frequency_ghz = get_cpu_freq_ghz(provider->get_cpuinfo_stream());

  get_load_and_process_stats(provider->get_loadavg_stream(), *this);

  // Call for Avg Top CPU processes
  get_top_processes(
      provider->get_top_mem_processes_avg_stream(),  // Input stream
      top_processes_avg_mem,                         // Output vector
      meminfo.total_kb                               // Total memory
  );
  get_top_processes(
      provider->get_top_cpu_processes_avg_stream(),  // Input stream
      top_processes_avg_cpu,                         // Output vector
      meminfo.total_kb                               // Total memory
  );

  //   // Call for Real Top CPU processes
  //   get_top_processes(
  //       provider->get_top_mem_processes_real_stream(),  // Input stream
  //       metrics.top_processes_real_mem,                // Output vector
  //       metrics.meminfo.total_kb                           // Total
  //       memory
  //   );
  //   get_top_processes(
  //       provider->get_top_cpu_processes_real_stream(),  // Input stream
  //       metrics.top_processes_real_cpu,                // Output vector
  //       metrics.meminfo.total_kb                           // Total
  //       memory
  //   );

  get_system_info(*this);
  return 0;
}
