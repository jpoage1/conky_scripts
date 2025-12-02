// processinfo.cpp
#include "processinfo.hpp"

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
std::istream& LocalDataStreams::get_top_mem_processes_avg_stream() {
  return create_stream_from_command(top_mem_procs, top_mem_processes_avg_cmd);
}
std::istream& ProcDataStreams::get_top_mem_processes_avg_stream() {
  return create_stream_from_command(top_mem_procs, top_mem_processes_avg_cmd);
};
std::istream& LocalDataStreams::get_top_cpu_processes_avg_stream() {
  return create_stream_from_command(top_mem_procs, top_cpu_processes_avg_cmd);
}
std::istream& ProcDataStreams::get_top_cpu_processes_avg_stream() {
  return create_stream_from_command(top_mem_procs, top_cpu_processes_avg_cmd);
}

/* Real Time */

std::istream& LocalDataStreams::get_top_mem_processes_real_stream() {
  return create_stream_from_command(top_mem_procs, top_mem_processes_real_cmd);
}
std::istream& ProcDataStreams::get_top_mem_processes_real_stream() {
  return create_stream_from_command(top_mem_procs, top_mem_processes_real_cmd);
};
std::istream& LocalDataStreams::get_top_cpu_processes_real_stream() {
  return create_stream_from_command(top_mem_procs, top_cpu_processes_real_cmd);
}
std::istream& ProcDataStreams::get_top_cpu_processes_real_stream() {
  return create_stream_from_command(top_mem_procs, top_cpu_processes_real_cmd);
}

void get_top_processes(std::istream& stream,
                       std::vector<ProcessInfo>& output_list,
                       long mem_total_kb) {
  output_list.clear();  // Clear the destination vector
  std::string line;
  double total_mem_kb =
      (mem_total_kb > 0) ? static_cast<double>(mem_total_kb) : 1.0;

  while (std::getline(stream, line)) {
    if (line.empty()) continue;

    std::stringstream ss(line);
    ProcessInfo proc;

    if (ss >> proc.pid >> proc.cpu_percent >> proc.vmRssKb) {
      //  Perform common calculations and parsing
      proc.mem_percent =
          (static_cast<double>(proc.vmRssKb) / total_mem_kb) * 100.0;

      // The rest of the line is the command name
      std::getline(ss, proc.name);

      // trim leading whitespace
      size_t first = proc.name.find_first_not_of(" \t");
      if (std::string::npos != first) {
        proc.name = proc.name.substr(first);
      } else {
        proc.name = "unknown";
      }

      output_list.push_back(proc);
    }
  }
}

void get_top_processes(std::vector<ProcessInfo>& output_list, long mem_total_kb,
                       bool sort_by_mem) {
  namespace fs = std::filesystem;
  output_list.clear();

  double total_mem_kb =
      (mem_total_kb > 0) ? static_cast<double>(mem_total_kb) : 1.0;

  for (const auto& entry : fs::directory_iterator("/proc")) {
    if (!entry.is_directory()) continue;

    const std::string pid_str = entry.path().filename().string();
    if (std::all_of(pid_str.begin(), pid_str.end(), ::isdigit)) {
      // 1. Read /proc/[pid]/status for RSS (memory) and name (cmdline)
      long pid = std::stol(pid_str);
      long vmRssKb = read_proc_status_field(entry.path().string(),
                                            "VmRSS");  // Read VmRSS directly

      if (vmRssKb <= 0) continue;  // Skip kernel threads or unreadable entries

      ProcessInfo proc;
      proc.pid = pid;
      proc.vmRssKb = vmRssKb;

      // 2. Calculate Memory Percent
      proc.mem_percent = (static_cast<double>(vmRssKb) / total_mem_kb) * 100.0;

      // 3. Get CPU Usage (Requires snapshotting/calculation - assumed
      // implemented elsewhere)
      proc.cpu_percent = get_process_cpu_usage(pid);

      // 4. Get Name (Simplified: Read command line or /proc/[pid]/comm)
      std::ifstream cmdline_file(entry.path() / "comm");
      std::getline(cmdline_file, proc.name);

      if (proc.name.empty()) {
        proc.name = "unknown";
      } else {
        // Remove trailing newline
        if (proc.name.back() == '\n') proc.name.pop_back();
      }

      output_list.push_back(std::move(proc));
    }
  }

  // 5. Final In-Memory Sorting and Head (Top 10)
  if (sort_by_mem) {
    std::sort(output_list.begin(), output_list.end(),
              [](const ProcessInfo& a, const ProcessInfo& b) {
                return a.vmRssKb > b.vmRssKb;
              });
  } else {
    std::sort(output_list.begin(), output_list.end(),
              [](const ProcessInfo& a, const ProcessInfo& b) {
                return a.cpu_percent > b.cpu_percent;
              });
  }

  // Keep only the top 10 results
  if (output_list.size() > 10) {
    output_list.resize(10);
  }
}
/**
 * @brief Reads a single numeric value (e.g., VmRSS in KiB) from a line in
 * /proc/[PID]/status.
 * * @param pid_dir The full path to the process directory (e.g., "/proc/1234").
 * @param field_name The specific field to search for (e.g., "VmRSS").
 * @return long The value found, typically in KiB. Returns 0 on error or if not
 * found.
 */
long read_proc_status_field(const std::string& pid_dir,
                            const std::string& field_name) {
  std::ifstream status_file(pid_dir + "/status");
  if (!status_file.is_open()) {
    return 0;
  }

  std::string line;
  // We search for "VmRSS:", etc.
  const std::string target_label = field_name + ":";

  while (std::getline(status_file, line)) {
    if (line.size() >= target_label.size() &&
        line.compare(0, target_label.size(), target_label) == 0) {
      // Found the target line. Use stringstream to parse value and unit.
      std::stringstream ss(line);
      std::string label;
      long value = 0;
      std::string unit;

      // Extracts the label ("VmRSS:"), the value (e.g., 1234), and the unit
      // ("kB")
      if (ss >> label >> value >> unit) {
        return value;
      }
    }
  }

  return 0;
}
long get_process_cumulative_cpu_jiffies(long pid) {
  std::string path = "/proc/" + std::to_string(pid) + "/stat";
  std::ifstream stat_file(path);
  if (!stat_file.is_open()) {
    return 0;
  }

  std::string line;
  std::getline(stat_file, line);
  std::stringstream ss(line);

  // /proc/stat structure requires skipping the first 13 fields (PID, comm,
  // state, etc.).
  std::string throwaway;
  for (int i = 0; i < 13; ++i) {
    ss >> throwaway;
  }

  long utime, stime;

  // Fields 14 (utime) and 15 (stime) contain CPU time in Jiffies.
  if (ss >> utime >> stime) {
    return utime + stime;
  }

  return 0;
}
// ProcessPollingTask.cpp

ProcessSnapshotMap ProcessPollingTask::read_data() {
  namespace fs = std::filesystem;
  ProcessSnapshotMap snapshots;

  // NOTE: This relies on the 'read_proc_status_field' and
  // 'get_process_cumulative_cpu_jiffies' helpers being available.

  for (const auto& entry : fs::directory_iterator("/proc")) {
    // 1. Filter for directories named only by digits (PIDs)
    const std::string pid_str = entry.path().filename().string();
    if (!entry.is_directory() ||
        !std::all_of(pid_str.begin(), pid_str.end(), ::isdigit)) {
      continue;
    }

    long pid = std::stol(pid_str);

    // 2. Read the necessary raw data fields
    ProcessRawSnapshot raw_snap;

    // Read memory (VmRSS)
    raw_snap.vmRssKb = read_proc_status_field(entry.path().string(), "VmRSS");

    // Read raw CPU time (jiffies)
    raw_snap.cumulative_cpu_jiffies = get_process_cumulative_cpu_jiffies(pid);

    // Read command name
    std::ifstream comm_file(entry.path() / "comm");
    std::getline(comm_file, raw_snap.name);

    if (raw_snap.vmRssKb > 0) {  // Only track active processes
      snapshots[pid] = raw_snap;
    }
  }
  return snapshots;
}
// ProcessPollingTask.cpp

void ProcessPollingTask::calculate(double time_delta_seconds) {
  // CRITICAL: Clear output vector before filling to prevent accumulation
  metrics.top_processes_avg_mem.clear();

  if (time_delta_seconds <= 0.0) return;

  // Use a factor to convert jiffies delta to a percentage
  // CLK_TCK must be retrieved once, e.g., via sysconf(_SC_CLK_TCK)
  static const long CLK_TCK = sysconf(_SC_CLK_TCK);
  double jiffies_per_sec = CLK_TCK * time_delta_seconds;

  if (jiffies_per_sec <= 0) return;

  // Iterate over the T2 snapshot and compare against T1
  for (const auto& [pid, current_snap] : t2_snapshots) {
    auto prev_it = t1_snapshots.find(pid);
    if (prev_it != t1_snapshots.end()) {
      const auto& prev_snap = prev_it->second;

      ProcessInfo info;
      info.pid = pid;
      info.name = current_snap.name;
      info.vmRssKb = current_snap.vmRssKb;  // Instantaneous metric

      // Calculate Jiffies Delta
      long jiffies_delta = current_snap.cumulative_cpu_jiffies -
                           prev_snap.cumulative_cpu_jiffies;

      // Calculate CPU Percentage
      if (jiffies_delta >= 0) {
        double usage =
            (static_cast<double>(jiffies_delta) / jiffies_per_sec) * 100.0;
        info.cpu_percent = std::min(usage, 100.0);
      } else {
        info.cpu_percent = 0.0;
      }

      // Calculate Memory Percentage
      info.mem_percent =
          (static_cast<double>(info.vmRssKb) / metrics.meminfo.total_kb) *
          100.0;

      metrics.top_processes_avg_mem.push_back(std::move(info));
    }
  }

  // Final sort (matching old ps behavior)
  std::sort(metrics.top_processes_avg_mem.begin(),
            metrics.top_processes_avg_mem.end(),
            [](const ProcessInfo& a, const ProcessInfo& b) {
              return b.cpu_percent < a.cpu_percent;  // Sort by CPU descending
            });

  // Resize to top 10
  if (metrics.top_processes_avg_mem.size() > 10) {
    metrics.top_processes_avg_mem.resize(10);
  }
}
double get_process_cpu_usage(long pid) {
  // Static map holds the last known CPU time and timestamp (T1) for all PIDs.
  static std::map<long, CpuState> previous_cpu_state;
  // Jiffies per second (system clock rate). Assumes this is constant.
  static const long CLK_TCK = sysconf(_SC_CLK_TCK);

  // T2 Snapshot (Current) - Relies on the user-provided jiffies reader
  long current_jiffies = get_process_cumulative_cpu_jiffies(pid);
  auto current_timestamp = std::chrono::steady_clock::now();

  // Check for previous T1 state
  auto it = previous_cpu_state.find(pid);

  if (it == previous_cpu_state.end()) {
    // First execution: Initialize T1 state and return 0 usage.
    previous_cpu_state[pid] = {current_jiffies, current_timestamp};
    return 0.0;
  }

  // T1 Snapshot (Previous)
  long last_jiffies = it->second.jiffies;
  auto last_timestamp = it->second.timestamp;

  // Calculate deltas
  long jiffies_delta = current_jiffies - last_jiffies;
  auto time_delta = current_timestamp - last_timestamp;
  double wall_time_seconds = std::chrono::duration<double>(time_delta).count();

  // Update T1 state for the next call
  it->second = {current_jiffies, current_timestamp};

  // Guard against errors (division by zero, negative time)
  if (wall_time_seconds <= 0 || jiffies_delta < 0 || CLK_TCK <= 0) {
    return 0.0;
  }

  // CPU % calculation
  double usage_percent =
      (static_cast<double>(jiffies_delta) / (wall_time_seconds * CLK_TCK)) *
      100.0;

  return std::min(usage_percent, 100.0);
}
