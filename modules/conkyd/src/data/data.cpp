#include "data.h"

#include <iomanip>

#include "corestat.h"
#include "cpuinfo.h"
#include "diskstat.h"
#include "meminfo.h"
#include "swapinfo.h"
#include "uptime.h"

SystemMetrics read_data(DataStreamProvider& provider) {
  SystemMetrics metrics;

  // This function (defined in previous turn) now returns the
  // std.vector<CoreStats> with all percentages calculated.
  // The 100ms sleep is inside calculate_cpu_usages.
  metrics.cores = calculate_cpu_usages(provider.get_stat_stream());

  // The rest of the data gathering:
  metrics.cpu_temp_c = provider.get_cpu_temperature();

  get_mem_usage(provider.get_meminfo_stream(), metrics.mem_used_kb,
                metrics.mem_total_kb, metrics.mem_percent);
  get_swap_usage(provider.get_meminfo_stream(), metrics.swap_used_kb,
                 metrics.swap_total_kb, metrics.swap_percent);

  metrics.uptime = get_uptime(provider.get_uptime_stream());
  metrics.cpu_frequency_ghz = get_cpu_freq_ghz(provider.get_cpuinfo_stream());
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
