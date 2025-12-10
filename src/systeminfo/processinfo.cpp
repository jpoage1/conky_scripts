// processinfo.cpp
#include "processinfo.hpp"

#include <unistd.h>

#include "data_local.hpp"
#include "polling.hpp"

namespace {

// Command: Get PID, RSS (in KiB), and command name.
// --no-headers simplifies parsing.
// --sort=-rss sorts by RSS descending.
// head -n 10 gets the top 10.
const char* top_mem_processes_avg_cmd =
    "ps -eo pid,%cpu,rss,comm --no-headers --sort=-rss | grep -v \" ps$\" | "
    "head "
    "-n 10";

const char* top_cpu_processes_avg_cmd =
    "ps -eo pid,%cpu,rss,comm --no-headers --sort=-%cpu | grep -v \" ps$\" | "
    "head -n 10";
const char* top_cpu_processes_real_cmd =
    "top -b -n 2 -d 0.9 -o %CPU | "  // 2 iterations, 0.9s apart, sorted by CPU
    "awk 'BEGIN{RS=\"\"} NR==2' | "  // Get only the second iteration's data
    "tail -n +8 | "                  // Skip the 7 header lines
    "head -n 10 | "                  // Get the top 10 processes
    "awk '{print $1, $9, $6, $12}'";
const char* top_mem_processes_real_cmd =
    "top -b -n 2 -d 0.9 -o %MEM | "
    "awk 'BEGIN{RS=\"\"} NR==2' | "
    "tail -n +8 | "
    "head -n 10 | "
    "awk '{print $1, $9, $6, $12}'";
};  // namespace

// --- HELPER FUNCTIONS ---

// Reads VmRSS from /proc/[pid]/status
long read_proc_vmrss(const std::string& pid_dir) {
  std::ifstream status_file(pid_dir + "/status");
  std::string line;
  while (std::getline(status_file, line)) {
    if (line.compare(0, 6, "VmRSS:") == 0) {
      std::stringstream ss(line);
      std::string label, unit;
      long value = 0;
      ss >> label >> value;
      return value;
    }
  }
  return 0;
}

// Helper to read /proc/[pid]/stat for Jiffies
long read_proc_jiffies(long pid) {
  std::string path = "/proc/" + std::to_string(pid) + "/stat";
  std::ifstream stat_file(path);
  if (!stat_file.is_open()) return 0;

  std::string line;
  std::getline(stat_file, line);
  std::stringstream ss(line);

  // Skip 13 fields to get to utime (14) and stime (15)
  std::string garbage;
  for (int i = 0; i < 13; ++i) ss >> garbage;

  long utime, stime;
  if (ss >> utime >> stime) return utime + stime;
  return 0;
}

// --- DATA PROVIDERS ---

// 1. LOCAL: High-performance direct /proc parsing
ProcessSnapshotMap LocalDataStreams::get_process_snapshots() {
  namespace fs = std::filesystem;
  ProcessSnapshotMap snapshots;

  for (const auto& entry : fs::directory_iterator("/proc")) {
    const std::string pid_str = entry.path().filename().string();

    // Fast check: is it a PID directory?
    if (!entry.is_directory() || !isdigit(pid_str[0])) continue;

    long pid = 0;
    try {
      pid = std::stol(pid_str);
    } catch (...) {
      continue;
    }

    ProcessRawSnapshot snap;
    snap.vmRssKb = read_proc_vmrss(entry.path().string());

    // Optimization: Don't read CPU/Name if memory is 0 (kernel threads often
    // have 0 RSS)
    if (snap.vmRssKb > 0) {
      snap.cumulative_cpu_time = read_proc_jiffies(pid);

      std::ifstream comm_file(entry.path() / "comm");
      std::getline(comm_file, snap.name);
      if (!snap.name.empty() && snap.name.back() == '\n') snap.name.pop_back();

      snapshots[pid] = snap;
    }
  }
  return snapshots;
}

// 2. REMOTE (SSH): Fallback using 'ps' command to reduce network overhead
ProcessSnapshotMap ProcDataStreams::get_process_snapshots() {
  ProcessSnapshotMap snapshots;

  // Command: Get PID, RSS(kb), Cumulative CPU Time(seconds), Command Name
  // 'times' gives cumulative user+system time in seconds on some
  // implementations, or we can use 'cputime'. We use 'times' which is usually
  // accumulator. Note: We multiply seconds by CLK_TCK (usually 100) to fake
  // "Jiffies" for consistency.
  std::string cmd = "ps -eo pid,rss,times,comm --no-headers";

  std::string output = execute_ssh_command(cmd.c_str());
  std::stringstream ss(output);
  std::string line;

  while (std::getline(ss, line)) {
    std::stringstream lss(line);
    long pid;
    ProcessRawSnapshot snap;
    long cpu_seconds = 0;

    if (lss >> pid >> snap.vmRssKb >> cpu_seconds) {
      // Fake the Jiffies: Seconds * 100.
      // This ensures the math in the Task (which divides by HZ) works for
      // both.
      snap.cumulative_cpu_time = cpu_seconds * 100;

      // Remainder of line is command
      std::getline(lss, snap.name);

      // Trim leading space from name
      size_t first = snap.name.find_first_not_of(" ");
      if (first != std::string::npos) snap.name = snap.name.substr(first);

      snapshots[pid] = snap;
    }
  }
  return snapshots;
}

// --- POLLING TASK LOGIC ---

ProcessPollingTask::ProcessPollingTask(DataStreamProvider& provider,
                                       SystemMetrics& metrics,
                                       MetricsContext& context)
    : IPollingTask(provider, metrics, context) {
  name = "Process polling";
}

void ProcessPollingTask::take_snapshot_1() { t1_snapshots = read_data(); }
void ProcessPollingTask::take_snapshot_2() { t2_snapshots = read_data(); }

ProcessSnapshotMap ProcessPollingTask::read_data() {
  return provider.get_process_snapshots();
}

void ProcessPollingTask::commit() {
  // Efficiently move T2 data to T1.
  // This clears T2 and prepares T1 for the next loop instantly.
  t1_snapshots = std::move(t2_snapshots);
}

void ProcessPollingTask::calculate(double time_delta_seconds) {
  // 1. Clear all destination vectors
  metrics.top_processes_avg_mem.clear();
  metrics.top_processes_avg_cpu.clear();
  metrics.top_processes_real_mem.clear();
  metrics.top_processes_real_cpu.clear();

  if (time_delta_seconds <= 0.0) return;

  static const long CLK_TCK = sysconf(_SC_CLK_TCK);
  double total_jiffies_available =
      static_cast<double>(CLK_TCK) * time_delta_seconds;

  std::vector<ProcessInfo> all_procs;
  all_procs.reserve(t2_snapshots.size());

  // 2. Calculate Real-Time Delta for ALL processes
  for (const auto& [pid, current_snap] : t2_snapshots) {
    auto prev_it = t1_snapshots.find(pid);
    if (prev_it != t1_snapshots.end()) {
      const auto& prev_snap = prev_it->second;

      ProcessInfo info;
      info.pid = pid;
      info.name = current_snap.name;
      info.vmRssKb = current_snap.vmRssKb;

      // --- CPU Calculation (Real-Time / Interval) ---
      // This calculates usage strictly for the window between Snapshot 1 and 2
      long jiffies_delta =
          current_snap.cumulative_cpu_time - prev_snap.cumulative_cpu_time;

      if (jiffies_delta >= 0 && total_jiffies_available > 0) {
        double usage =
            (static_cast<double>(jiffies_delta) / total_jiffies_available) *
            100.0;
        info.cpu_percent = std::min(usage, 100.0);
      } else {
        info.cpu_percent = 0.0;
      }

      // --- Memory Calculation ---
      if (metrics.meminfo.total_kb > 0) {
        info.mem_percent =
            (static_cast<double>(info.vmRssKb) / metrics.meminfo.total_kb) *
            100.0;
      }

      all_procs.push_back(std::move(info));
    }
  }

  // Helper lambda for sorting and assigning top 10
  auto populate_top_10 = [](std::vector<ProcessInfo>& source,
                            std::vector<ProcessInfo>& dest, bool sort_by_mem) {
    dest.reserve(10);
    if (sort_by_mem) {
      // Sort by Memory (RSS)
      if (source.size() > 10) {
        std::partial_sort(source.begin(), source.begin() + 10, source.end(),
                          [](const ProcessInfo& a, const ProcessInfo& b) {
                            return a.vmRssKb > b.vmRssKb;
                          });
        dest.assign(source.begin(), source.begin() + 10);
      } else {
        std::sort(source.begin(), source.end(),
                  [](const ProcessInfo& a, const ProcessInfo& b) {
                    return a.vmRssKb > b.vmRssKb;
                  });
        dest = source;
      }
    } else {
      // Sort by CPU
      if (source.size() > 10) {
        std::partial_sort(source.begin(), source.begin() + 10, source.end(),
                          [](const ProcessInfo& a, const ProcessInfo& b) {
                            return a.cpu_percent > b.cpu_percent;
                          });
        dest.assign(source.begin(), source.begin() + 10);
      } else {
        std::sort(source.begin(), source.end(),
                  [](const ProcessInfo& a, const ProcessInfo& b) {
                    return a.cpu_percent > b.cpu_percent;
                  });
        dest = source;
      }
    }
  };

  // 3. Populate Vectors
  // Memory: Real and Avg are usually identical (Current RSS)
  populate_top_10(all_procs, metrics.top_processes_real_mem, true);
  metrics.top_processes_avg_mem = metrics.top_processes_real_mem;

  // CPU: This math produces REAL (Live) stats, so we assign to _real_cpu
  populate_top_10(all_procs, metrics.top_processes_real_cpu, false);

  // Note: metrics.top_processes_avg_cpu (Lifetime Average) cannot be
  // calculated here without Process Start Time (from /proc/[pid]/stat field 22)
  // and System Uptime. For now, it remains empty or you can alias it to Real.
}
