// corestat.cpp
#include "corestat.hpp"

#include "data_local.hpp"
#include "data_ssh.hpp"
#include "polling.hpp"

std::istream& LocalDataStreams::get_stat_stream() {
  return create_stream_from_file(stat, "/proc/stat");
}

std::istream& ProcDataStreams::get_stat_stream() {
  return create_stream_from_command(stat, "cat /proc/stat");
}

CpuPollingTask::CpuPollingTask(DataStreamProvider& provider,
                               SystemMetrics& metrics, MetricsContext& context)
    : IPollingTask(provider, metrics, context) {
  //   dump_fstream(provider.get_stat_stream());
}

void CpuPollingTask::calculate() {
  std::vector<CoreStats> all_core_stats;
  size_t num_cores_and_agg =
      std::min(prev_snapshots.size(), current_snapshots.size());

  for (size_t i = 0; i < num_cores_and_agg; ++i) {
    const auto& prev = prev_snapshots[i];
    const auto& curr = current_snapshots[i];

    unsigned long long total_delta =
        curr.get_total_time() - prev.get_total_time();

    if (total_delta == 0) {
      all_core_stats.push_back({i, 0.0f, 0.0f, 0.0f, 0.0f, 100.0f, 0.0f});
      continue;
    }

    unsigned long long user_delta = curr.user - prev.user;
    unsigned long long nice_delta = curr.nice - prev.nice;
    unsigned long long system_delta = curr.system - prev.system;
    unsigned long long iowait_delta = curr.iowait - prev.iowait;
    unsigned long long idle_delta = curr.idle - prev.idle;

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

void CpuPollingTask::take_initial_snapshot() {
  //   dump_fstream(provider.get_stat_stream());
  set_timestamp();
  prev_snapshots = read_data(provider.get_stat_stream());
}

void CpuPollingTask::take_new_snapshot() {
  set_delta_time();
  current_snapshots = read_data(provider.get_stat_stream());
}

void CpuPollingTask::commit() { prev_snapshots = current_snapshots; }

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
