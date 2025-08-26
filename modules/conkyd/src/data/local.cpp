#include "local.h"

#include <istream>

#include "corestat.h"
#include "cpuinfo.h"
#include "diskstat.h"
#include "meminfo.h"
#include "swapinfo.h"
#include "uptime.h"

SystemMetrics read_data(std::ifstream &cpu_file_stream,
                        std::ifstream &meminfo_file_stream,
                        std::ifstream &uptime_file_stream,
                        std::ifstream &stat_file_stream) {
  SystemMetrics metrics;
  auto cores = read_cpu_times(stat_file_stream);
  for (size_t i = 0; i < cores.size(); ++i) {
    metrics.cores.push_back({i, cores[i].idle_time, cores[i].total_time});
  }

  get_mem_usage(meminfo_file_stream, metrics.mem_used_kb, metrics.mem_total_kb,
                metrics.mem_percent);
  get_swap_usage(meminfo_file_stream, metrics.swap_used_kb,
                 metrics.swap_total_kb, metrics.swap_percent);

  metrics.uptime = get_uptime(uptime_file_stream);
  metrics.cpu_frequency_ghz = get_cpu_freq_ghz(cpu_file_stream);
  return metrics;
}

SystemMetrics read_data(LocalDataStreams &streams) {
  SystemMetrics metrics;
  auto cores = read_cpu_times(streams.stat);  // Line 34
  for (size_t i = 0; i < cores.size(); ++i) {
    metrics.cores.push_back({i, cores[i].idle_time, cores[i].total_time});
  }

  get_mem_usage(streams.meminfo, metrics.mem_used_kb, metrics.mem_total_kb,
                metrics.mem_percent);
  get_swap_usage(streams.meminfo, metrics.swap_used_kb, metrics.swap_total_kb,
                 metrics.swap_percent);

  metrics.uptime = get_uptime(streams.uptime);
  metrics.cpu_frequency_ghz = get_cpu_freq_ghz(streams.cpuinfo);
  return metrics;
}

LocalDataStreams get_local_file_streams() {
  LocalDataStreams streams;
  streams.cpuinfo.open("/proc/cpuinfo");
  streams.meminfo.open("/proc/meminfo");
  streams.uptime.open("/proc/uptime");
  streams.stat.open("/proc/stat");
  return streams;
}
