#include "corestat.h"
#include "cpuinfo.h"
#include "diskstat.h"
#include "meminfo.h"
#include "swapinfo.h"
#include "uptime.h"

int main() {
  std::cout << "CPU Frequency Ghz " << get_cpu_freq_ghz() << std::endl;

  auto cores = read_cpu_times();
  for (size_t i = 0; i < cores.size(); ++i) {
    std::cout << "Core " << i << ": idle=" << cores[i].idle_time
              << " total=" << cores[i].total_time << std::endl;
  }

  std::cout << "CPU Usage: " << std::endl;

  // auto cores = read_cpu_times();
  // for (size_t i = 0; i < cores.size(); ++i) {
  //   std::cout << format_cpu_times(cores[i], i) << std::endl;
  // }

  std::cout << "Uptime: " << get_uptime() << std::endl;

  long mem_used, mem_total;
  int mem_percent;
  get_mem_usage(mem_used, mem_total, mem_percent);
  std::cout << "Mem: " << mem_used << " / " << mem_total << " kB ("
            << mem_percent << "%)" << std::endl;

  long swap_used, swap_total;
  int swap_percent;
  get_swap_usage(swap_used, swap_total, swap_percent);
  std::cout << "Swap: " << swap_used << " / " << swap_total << " kB ("
            << swap_percent << "%)" << std::endl;
  return 0;
}
