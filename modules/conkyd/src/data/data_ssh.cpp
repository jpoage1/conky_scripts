#include "data_ssh.h"

#include "corestat.h"
#include "cpuinfo.h"
#include "diskstat.h"
#include "meminfo.h"
#include "swapinfo.h"
#include "uptime.h"

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

SystemMetrics read_data(ProcDataStreams &streams) {
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
