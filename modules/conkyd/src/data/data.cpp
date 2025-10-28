#include "data.h"

#include <sys/utsname.h>

#include <chrono>
#include <iomanip>
#include <map>

#include "corestat.h"
#include "cpuinfo.h"
#include "diskstat.h"
#include "meminfo.h"
#include "networkstats.hpp"
#include "swapinfo.h"
#include "uptime.h"

void rewind(std::stringstream& stream, const std::string& streamName) {
  // Debug: Check if stream is in a bad state before attempting reset
  if (stream.fail() || stream.bad()) {
    std::cerr << "DEBUG: Stream '" << streamName
              << "' was in fail/bad state before rewind." << std::endl;
  }

  stream.clear();
  stream.seekg(0, std::ios::beg);

  // Debug: Confirm stream is usable after reset
  if (stream.fail() || stream.bad()) {
    std::cerr << "DEBUG: Stream '" << streamName
              << "' is still in fail/bad state after rewind. FATAL."
              << std::endl;
  }
}

SystemMetrics read_data(DataStreamProvider& provider) {
  SystemMetrics metrics;

  metrics.cores = calculate_cpu_usages(provider.get_stat_stream());

  metrics.cpu_temp_c = provider.get_cpu_temperature();

  get_mem_usage(provider.get_meminfo_stream(), metrics.mem_used_kb,
                metrics.mem_total_kb, metrics.mem_percent);
  get_swap_usage(provider.get_meminfo_stream(), metrics.swap_used_kb,
                 metrics.swap_total_kb, metrics.swap_percent);

  metrics.uptime = get_uptime(provider.get_uptime_stream());
  metrics.cpu_frequency_ghz = get_cpu_freq_ghz(provider.get_cpuinfo_stream());

  get_load_and_process_stats(provider, metrics);
  get_network_stats(provider, metrics);

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

  return metrics;
}

void print_metrics(const SystemMetrics& metrics) {
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
}
void get_load_and_process_stats(DataStreamProvider& provider,
                                SystemMetrics& metrics) {
  // 1. Get Load Average
  provider.get_loadavg_stream() >> metrics.load_avg_1m >> metrics.load_avg_5m >>
      metrics.load_avg_15m;

  // 2. Get Process Counts
  std::istream& stat_stream = provider.get_stat_stream();
  stat_stream.clear();
  stat_stream.seekg(0, std::ios::beg);
  std::string line;
  while (std::getline(stat_stream, line)) {
    if (line.rfind("processes", 0) == 0) {
      std::stringstream ss(line);
      std::string key;
      ss >> key >> metrics.processes_total;
    } else if (line.rfind("procs_running", 0) == 0) {
      std::stringstream ss(line);
      std::string key;
      ss >> key >> metrics.processes_running;
    } else if (line.rfind("procs_blocked", 0) == 0) {
      // All process lines are together, so we can stop
      break;
    }
  }
}
