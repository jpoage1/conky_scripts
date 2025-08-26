#include "data.h"

#include "corestat.h"
#include "cpuinfo.h"
#include "diskstat.h"
#include "meminfo.h"
#include "swapinfo.h"
#include "uptime.h"

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
