#include "corestat.h"

#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

std::vector<CPUCore> read_cpu_times(std::istream& input_stream) {
  std::vector<CPUCore> cores;
  std::string line;
  while (std::getline(input_stream, line)) {
    if (line.compare(0, 3, "cpu") != 0 || line.compare(0, 4, "cpu ") == 0)
      continue;
    std::istringstream ss(line);
    std::string label;
    ss >> label;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    unsigned long long idle_all = idle + iowait;
    unsigned long long total =
        user + nice + system + idle + iowait + irq + softirq + steal;
    cores.push_back({idle_all, total});
  }
  return cores;
}

std::vector<float> get_cpu_usages(std::istream& input_stream) {
  auto t1 = read_cpu_times(input_stream);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  auto t2 = read_cpu_times(input_stream);
  std::vector<float> usages;
  for (size_t i = 0; i < t1.size(); ++i) {
    unsigned long long idle_diff = t2[i].idle_time - t1[i].idle_time;
    unsigned long long total_diff = t2[i].total_time - t1[i].total_time;
    float usage =
        total_diff == 0 ? 0 : 100.0f * (total_diff - idle_diff) / total_diff;
    usages.push_back(usage);
  }
  return usages;
}

std::string format_cpu_times(const CPUCore& core, size_t index) {
  double idlePercent = 100.0 * core.idle_time / core.total_time;
  double usagePercent = 100.0 - idlePercent;
  return "Core " + std::to_string(index) +
         ": Idle=" + std::to_string(core.idle_time) +
         ", Total=" + std::to_string(core.total_time) +
         ", Usage=" + std::to_string(usagePercent) + "%";
}
