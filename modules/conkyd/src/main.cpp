#include <istream>

#include "data.h"

int main() {
  std::ifstream cpu_file_stream("/proc/cpuinfo");
  std::ifstream meminfo_file_stream("/proc/meminfo");
  std::ifstream uptime_file_stream("/proc/uptime");
  std::ifstream stat_file_stream("/proc/stat");

  SystemMetrics metrics = read_data(cpu_file_stream, meminfo_file_stream,
                                    uptime_file_stream, stat_file_stream);
  print_metrics(metrics);
  return 0;
}
