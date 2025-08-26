#include "main.h"

#include <istream>

#include "corestat.h"
#include "cpuinfo.h"
#include "diskstat.h"
#include "meminfo.h"
#include "ssh.h"
#include "swapinfo.h"
#include "uptime.h"

int main(int argc, char *argv[]) {
  bool use_ssh = false;

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--ssh" || std::string(argv[i]) == "-s") {
      use_ssh = true;
      break;
    }
  }

  DataStreamProvider *provider = nullptr;

  if (use_ssh) {
    if (setup_ssh_session() != 0) {
      std::cerr << "Failed to set up SSH session. Exiting." << std::endl;
      return 1;
    }
    ProcDataStreams ssh_streams = get_ssh_streams();
    provider = &ssh_streams;
  } else {
    LocalDataStreams local_streams = get_local_file_streams();
    provider = &local_streams;
  }

  if (provider) {
    SystemMetrics metrics = read_data(*provider);
    print_metrics(metrics);
  } else {
    std::cerr << "Error: No data provider could be initialized." << std::endl;
    return 1;
  }
  if (use_ssh) {
    cleanup_ssh_session();
  }

  return 0;
}
SystemMetrics read_data(DataStreamProvider &provider) {
  SystemMetrics metrics;

  auto cores = read_cpu_times(provider.get_stat_stream());
  for (size_t i = 0; i < cores.size(); ++i) {
    metrics.cores.push_back({i, cores[i].idle_time, cores[i].total_time});
  }

  get_mem_usage(provider.get_meminfo_stream(), metrics.mem_used_kb,
                metrics.mem_total_kb, metrics.mem_percent);
  get_swap_usage(provider.get_meminfo_stream(), metrics.swap_used_kb,
                 metrics.swap_total_kb, metrics.swap_percent);

  metrics.uptime = get_uptime(provider.get_uptime_stream());
  metrics.cpu_frequency_ghz = get_cpu_freq_ghz(provider.get_cpuinfo_stream());
  return metrics;
}

ProcDataStreams get_ssh_streams() {
  // Get the remote data
  std::string cpu_data = execute_ssh_command("cat /proc/cpuinfo");
  std::string meminfo_data = execute_ssh_command("cat /proc/meminfo");
  std::string uptime_data = execute_ssh_command("cat /proc/uptime");
  std::string stat_data = execute_ssh_command("cat /proc/stat");

  // Create string streams from the retrieved data
  std::stringstream cpu_file_stream(cpu_data);
  std::stringstream meminfo_file_stream(meminfo_data);
  std::stringstream uptime_file_stream(uptime_data);
  std::stringstream stat_file_stream(stat_data);

  ProcDataStreams streams;
  streams.cpuinfo << cpu_data;
  streams.meminfo << meminfo_data;
  streams.uptime << uptime_data;
  streams.stat << stat_data;

  return streams;
}
LocalDataStreams get_local_file_streams() {
  LocalDataStreams streams;
  streams.cpuinfo.open("/proc/cpuinfo");
  streams.meminfo.open("/proc/meminfo");
  streams.uptime.open("/proc/uptime");
  streams.stat.open("/proc/stat");
  return streams;
}

void print_metrics(SystemMetrics metrics) {
  std::cout << "CPU Frequency Ghz " << metrics.cpu_frequency_ghz << std::endl;

  for (const auto &core : metrics.cores) {
    std::cout << "Core " << core.core_id << ": idle=" << core.idle_time
              << " total=" << core.total_time << std::endl;
  }

  std::cout << "CPU Usage: " << std::endl;

  // auto cores = read_cpu_times();
  // for (size_t i = 0; i < cores.size(); ++i) {
  //   std::cout << format_cpu_times(cores[i], i) << std::endl;
  // }

  std::cout << "Uptime: " << metrics.uptime << std::endl;

  std::cout << "Mem: " << metrics.mem_used_kb << " / " << metrics.mem_total_kb
            << " kB (" << metrics.mem_percent << "%)" << std::endl;

  std::cout << "Swap: " << metrics.swap_used_kb << " / "
            << metrics.swap_total_kb << " kB (" << metrics.swap_percent << "%)"
            << std::endl;
}
