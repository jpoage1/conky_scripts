#include "data.h"

#include <sys/utsname.h>

#include <chrono>
#include <chrono>  // For seconds
#include <iomanip>
#include <map>
#include <memory>  // For std::unique_ptr
#include <thread>  // For sleep_for
#include <type_traits>
#include <vector>

#include "corestat.h"
#include "cpu_processes.hpp"
#include "cpuinfo.h"
#include "diskstat.h"
#include "hwmonitor.hpp"
#include "load_avg.hpp"
#include "mem_processes.hpp"
#include "meminfo.h"
#include "metrics.hpp"
#include "networkstats.hpp"
#include "parser.hpp"
#include "processinfo.hpp"
#include "uptime.hpp"

PollingTaskList read_data(DataStreamProvider& provider,
                          SystemMetrics& metrics) {
  PollingTaskList polling_tasks;
  polling_tasks.push_back(std::make_unique<CpuPollingTask>(provider, metrics));
  polling_tasks.push_back(
      std::make_unique<NetworkPollingTask>(provider, metrics));
  // polling_tasks.push_back(std::make_unique<DiskPollingTask>(provider,
  // metrics));

  metrics.cpu_temp_c = provider.get_cpu_temperature();

  get_mem_usage(provider.get_meminfo_stream(), metrics.mem_used_kb,
                metrics.mem_total_kb, metrics.mem_percent);
  get_swap_usage(provider.get_meminfo_stream(), metrics.swap_used_kb,
                 metrics.swap_total_kb, metrics.swap_percent);

  metrics.uptime = get_uptime(provider.get_uptime_stream());
  metrics.cpu_frequency_ghz = get_cpu_freq_ghz(provider.get_cpuinfo_stream());

  get_load_and_process_stats(provider, metrics);
  get_top_processes_mem(provider, metrics);
  get_top_processes_cpu(provider, metrics);

  get_system_info(metrics);
  return polling_tasks;
}

void get_system_info(SystemMetrics& metrics) {
  struct utsname uts_info;
  if (uname(&uts_info) == 0) {  // 0 indicates success
    metrics.sys_name = uts_info.sysname;
    metrics.node_name = uts_info.nodename;
    metrics.kernel_release = uts_info.release;
    metrics.machine_type = uts_info.machine;
  } else {
    // Handle uname error if needed, e.g., set default strings
    metrics.sys_name = "N/A";
    metrics.node_name = "N/A";
    metrics.kernel_release = "N/A";
    metrics.machine_type = "N/A";
  }
}

void print_device_metrics(const std::vector<DeviceInfo>& devices) {
  extern std::tuple<std::string, std::function<FuncType>> conky_columns[];
  extern const size_t CONKY_COLUMNS_COUNT;
  std::cout.setf(std::ios::unitbuf);

  print_column_headers(conky_columns, CONKY_COLUMNS_COUNT);
  print_rows(devices, CONKY_COLUMNS_COUNT);
}

void print_metrics(const CombinedMetrics& metrics) {
  print_system_metrics(metrics.system);
  print_device_metrics(metrics.disks);
}
void print_metrics(const SystemMetrics& metrics) {
  print_system_metrics(metrics);
}
void print_system_metrics(const SystemMetrics& metrics) {
  // Set precision for floating point numbers (percentages, temp, freq)
  std::cout << std::fixed << std::setprecision(1);

  std::cout << "CPU Frequency Ghz: " << metrics.cpu_frequency_ghz << std::endl;
  std::cout << "CPU Temp C: " << metrics.cpu_temp_c << " C" << std::endl;

  std::cout << "--- CPU Usage ---" << std::endl;
  // Loop over the vector of CoreStats (which now contains percentages)
  for (const auto& core : metrics.cores) {
    std::cout << "  Core " << std::setw(2) << core.core_id << ": "
              << std::setw(5) << core.total_usage_percent << "% "
              << "(User: " << std::setw(5) << core.user_percent << "%, "
              << "Sys: " << std::setw(5) << core.system_percent << "%, "
              << "IOWait: " << std::setw(5) << core.iowait_percent << "%)"
              << std::endl;
  }
  std::cout << "-----------------" << std::endl;

  std::cout << "Uptime: " << metrics.uptime << std::endl;

  std::cout << "Mem: " << metrics.mem_used_kb << " / " << metrics.mem_total_kb
            << " kB (" << metrics.mem_percent << "%)" << std::endl;

  std::cout << "Swap: " << metrics.swap_used_kb << " / "
            << metrics.swap_total_kb << " kB (" << metrics.swap_percent << "%)"
            << std::endl;

  std::cout << "--- Top Processes (Mem) ---" << std::endl;
  std::cout << "PID\tVmRSS (MiB)\tName" << std::endl;
  for (const auto& proc : metrics.top_processes_mem) {
    double vmRssMiB = static_cast<double>(proc.vmRssKb) / 1024.0;
    std::cout << proc.pid << "\t" << std::fixed << std::setprecision(1)
              << vmRssMiB << "\t\t" << proc.name << std::endl;
  }
  std::cout << "---------------------------" << std::endl;
  std::cout << "--- Top Processes (CPU) ---" << std::endl;
  std::cout << "PID\t%CPU\t\tName" << std::endl;
  // Iterate over the new vector, accessing the cpu_percent field
  for (const auto& proc : metrics.top_processes_cpu) {
    std::cout << proc.pid << "\t" << std::fixed << std::setprecision(1)
              << proc.cpu_percent << "%\t\t" << proc.name << std::endl;
  }
  std::cout << "---------------------------" << std::endl;
}
