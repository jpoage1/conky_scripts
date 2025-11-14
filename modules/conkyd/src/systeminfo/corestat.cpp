// corestat.cpp
#include "corestat.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "data_local.h"
#include "data_ssh.h"

std::istream& LocalDataStreams::get_stat_stream() {
  reset_stream(stat, "/proc/stat");
  return stat;
}

std::istream& ProcDataStreams::get_stat_stream() {
  std::string stat_data = execute_ssh_command("cat /proc/stat");
  stat.str(stat_data);
  return stat;
}

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

std::string format_cpu_times(const CPUCore& core, size_t index) {
  double idlePercent = 100.0 * core.idle_time / core.total_time;
  double usagePercent = 100.0 - idlePercent;
  return "Core " + std::to_string(index) +
         ": Idle=" + std::to_string(core.idle_time) +
         ", Total=" + std::to_string(core.total_time) +
         ", Usage=" + std::to_string(usagePercent) + "%";
}
std::vector<CpuSnapshot> read_cpu_snapshots(std::istream& input_stream) {
  std::vector<CpuSnapshot> snapshots;
  std::string line;

  input_stream.clear();
  input_stream.seekg(0, std::ios::beg);

  while (std::getline(input_stream, line)) {
    if (line.compare(0, 3, "cpu") != 0) {
      break;
    }

    std::istringstream ss(line);
    std::string label;
    ss >> label;

    CpuSnapshot snap;
    ss >> snap.user >> snap.nice >> snap.system >> snap.idle >> snap.iowait >>
        snap.irq >> snap.softirq >> snap.steal;
    snapshots.push_back(snap);
  }
  return snapshots;
}
/*
// DEPRECATED: Polling logic has been centralized in poll_dynamic_stats().
// These functions perform their own sleep and are no longer used.
*/
std::vector<float> get_cpu_usages(std::istream& input_stream) {
  auto t1 = read_cpu_times(input_stream);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  auto t2 = read_cpu_times(input_stream);
  return get_cpu_usages(t1, t2);
}

/*
// DEPRECATED: Polling logic has been centralized in poll_dynamic_stats().
// These functions perform their own sleep and are no longer used.
*/
std::vector<CoreStats> calculate_cpu_usages(std::istream& input_stream) {
  auto t1_snapshots = read_cpu_snapshots(input_stream);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  auto t2_snapshots = read_cpu_snapshots(input_stream);
  return calculate_cpu_usages(t1_snapshots, t2_snapshots);
}

std::vector<float> get_cpu_usages(const std::vector<CPUCore>& t1,
                                  const std::vector<CPUCore>& t2) {
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

std::vector<CoreStats> calculate_cpu_usages(
    const std::vector<CpuSnapshot>& t1_snapshots,
    const std::vector<CpuSnapshot>& t2_snapshots) {
  std::vector<CoreStats> all_core_stats;
  size_t num_cores_and_agg = std::min(t1_snapshots.size(), t2_snapshots.size());

  for (size_t i = 0; i < num_cores_and_agg; ++i) {
    const auto& t1 = t1_snapshots[i];
    const auto& t2 = t2_snapshots[i];

    unsigned long long total_delta = t2.get_total_time() - t1.get_total_time();

    if (total_delta == 0) {
      all_core_stats.push_back({i, 0.0f, 0.0f, 0.0f, 0.0f, 100.0f, 0.0f});
      continue;
    }

    unsigned long long user_delta = t2.user - t1.user;
    unsigned long long nice_delta = t2.nice - t1.nice;
    unsigned long long system_delta = t2.system - t1.system;
    unsigned long long iowait_delta = t2.iowait - t1.iowait;
    unsigned long long idle_delta = t2.idle - t1.idle;

    CoreStats core_stat;
    core_stat.core_id = i;  // 0 = aggregate, 1 = Core 0, etc.

    core_stat.user_percent = 100.0f * user_delta / total_delta;
    core_stat.nice_percent = 100.0f * nice_delta / total_delta;
    core_stat.system_percent = 100.0f * system_delta / total_delta;
    core_stat.iowait_percent = 100.0f * iowait_delta / total_delta;
    core_stat.idle_percent = 100.0f * idle_delta / total_delta;

    // Total usage ("CPU Load") = user + nice + system
    core_stat.total_usage_percent = core_stat.user_percent +
                                    core_stat.nice_percent +
                                    core_stat.system_percent;

    all_core_stats.push_back(core_stat);
  }
  return all_core_stats;
}
