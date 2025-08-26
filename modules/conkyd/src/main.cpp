#include <istream>

#include "corestat.h"
#include "cpuinfo.h"
#include "diskstat.h"
#include "meminfo.h"
#include "ssh.h"
#include "swapinfo.h"
#include "uptime.h"

int main() {
  ssh_connection();

  std::ifstream cpu_file_stream("/proc/cpuinfo");
  std::ifstream meminfo_file_stream("/proc/meminfo");
  std::ifstream uptime_file_stream("/proc/uptime");
  std::ifstream stat_file_stream("/proc/stat");

  SystemMetrics metrics = read_data(cpu_file_stream, meminfo_file_stream,
                                    uptime_file_stream, stat_file_stream);
  print_data(metrics);
  return 0;
}

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

void print_data(SystemMetrics metrics) {
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
