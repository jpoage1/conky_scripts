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

CpuPollingTask::CpuPollingTask(DataStreamProvider& _provider,
                               SystemMetrics& _metrics)
    : IPollingTask(_provider, _metrics) {}

void CpuPollingTask::calculate(double /*time_delta_seconds*/) {
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
  metrics.cores = all_core_stats;
}

void CpuPollingTask::take_snapshot_1() {
  t1_snapshots = read_data(provider.get_stat_stream());
}

void CpuPollingTask::take_snapshot_2() {
  t2_snapshots = read_data(provider.get_stat_stream());
}

CpuSnapshotList CpuPollingTask::read_data(std::istream& input_stream) {
  CpuSnapshotList snapshots;
  std::string line;

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

/* Deprecated, possibly dead code */
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

/* Deprecated, possibly dead code */
std::string format_cpu_times(const CPUCore& core, size_t index) {
  double idlePercent = 100.0 * core.idle_time / core.total_time;
  double usagePercent = 100.0 - idlePercent;
  return "Core " + std::to_string(index) +
         ": Idle=" + std::to_string(core.idle_time) +
         ", Total=" + std::to_string(core.total_time) +
         ", Usage=" + std::to_string(usagePercent) + "%";
}
