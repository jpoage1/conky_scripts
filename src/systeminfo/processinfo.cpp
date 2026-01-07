// processinfo.cpp
#include "processinfo.hpp"

#include <sys/stat.h>
#include <unistd.h>

#include "context.hpp"
#include "data_local.hpp"
#include "data_ssh.hpp"
#include "diskstat.hpp"
#include "log.hpp"
#include "lua_generator.hpp"
#include "metrics.hpp"
#include "polling.hpp"

namespace telemetry {

// namespace {

// // Command: Get PID, RSS (in KiB), and command name.
// // --no-headers simplifies parsing.
// // --sort=-rss sorts by RSS descending.
// // head -n 10 gets the top 10.
// const char* top_mem_processes_avg_cmd =
//     "ps -eo pid,%cpu,rss,comm --no-headers --sort=-rss | grep -v \" ps$\" | "
//     "head "
//     "-n 10";

// const char* top_cpu_processes_avg_cmd =
//     "ps -eo pid,%cpu,rss,comm --no-headers --sort=-%cpu | grep -v \" ps$\" |
//     " "head -n 10";
// const char* top_cpu_processes_real_cmd =
//     "top -b -n 2 -d 0.9 -o %CPU | "  // 2 iterations, 0.9s apart, sorted by
//     CPU "awk 'BEGIN{RS=\"\"} NR==2' | "  // Get only the second iteration's
//     data "tail -n +8 | "                  // Skip the 7 header lines "head -n
//     10 | "                  // Get the top 10 processes "awk '{print $1, $9,
//     $6, $12}'";
// const char* top_mem_processes_real_cmd =
//     "top -b -n 2 -d 0.9 -o %MEM | "
//     "awk 'BEGIN{RS=\"\"} NR==2' | "
//     "tail -n +8 | "
//     "head -n 10 | "
//     "awk '{print $1, $9, $6, $12}'";
// };  // namespace

// --- HELPER FUNCTIONS ---

// Reads VmRSS from /proc/[pid]/status
long read_proc_vmrss(const std::string &pid_dir) {
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
  if (!stat_file.is_open())
    return 0;

  std::string line;
  std::getline(stat_file, line);
  std::stringstream ss(line);

  // Skip 13 fields to get to utime (14) and stime (15)
  std::string garbage;
  for (int i = 0; i < 13; ++i)
    ss >> garbage;

  long utime, stime;
  if (ss >> utime >> stime)
    return utime + stime;
  return 0;
}
// Returns pair: {cumulative_jiffies, start_time_jiffies}
std::pair<long, unsigned long long> read_proc_stat_values(long pid) {
  std::string path = "/proc/" + std::to_string(pid) + "/stat";
  std::ifstream stat_file(path);
  if (!stat_file.is_open())
    return {0, 0};

  std::string line;
  std::getline(stat_file, line);

  // Fast-forward past the command name (which is in parens) to handle spaces
  size_t last_paren = line.find_last_of(')');
  if (last_paren == std::string::npos)
    return {0, 0};

  std::stringstream data_ss(line.substr(last_paren + 1));
  std::string garbage;

  // Skip fields 3-13 to get to utime(14)
  for (int i = 0; i < 11; ++i)
    data_ss >> garbage;

  long utime, stime;
  data_ss >> utime >> stime; // Fields 14, 15

  // Skip fields 16-21 to get to starttime(22)
  for (int i = 0; i < 6; ++i)
    data_ss >> garbage;

  unsigned long long starttime;
  data_ss >> starttime; // Field 22

  return {utime + stime, starttime};
}
long get_system_uptime_jiffies() {
  std::ifstream uptime_file("/proc/uptime");
  double uptime_seconds;
  if (uptime_file >> uptime_seconds) {
    static const long CLK_TCK = sysconf(_SC_CLK_TCK);
    return static_cast<long>(uptime_seconds * CLK_TCK);
  }
  return 0;
}

// --- DATA PROVIDERS ---

// 1. LOCAL: High-performance direct /proc parsing
ProcessSnapshotMap
LocalDataStreams::get_process_snapshots(bool only_user_processes) {
  namespace fs = std::filesystem;
  ProcessSnapshotMap snapshots;

  for (const auto &entry : fs::directory_iterator("/proc")) {
    const std::string pid_str = entry.path().filename().string();

    // Fast check: is it a PID directory?
    if (!entry.is_directory() || !isdigit(pid_str[0]))
      continue;

    struct stat stats;
    if (::stat(entry.path().c_str(), &stats) == 0) {
      if (only_user_processes && stats.st_uid != getuid()) {
        continue; // Skip processes not owned by me
      }
    }

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
      auto stat_vals = read_proc_stat_values(pid);
      snap.cumulative_cpu_time = stat_vals.first;
      snap.start_time = stat_vals.second;

      std::ifstream comm_file(entry.path() / "comm");
      std::getline(comm_file, snap.name);
      if (!snap.name.empty() && snap.name.back() == '\n')
        snap.name.pop_back();

      snapshots[pid] = snap;
    }
  }
  return snapshots;
}

// 2. REMOTE (SSH): Fallback using 'ps' command to reduce network overhead
ProcessSnapshotMap
ProcDataStreams::get_process_snapshots(bool /*only_user_processes*/) {
  ProcessSnapshotMap snapshots;

  // Command: Get PID, RSS(kb), Cumulative CPU Time(seconds), Command Name
  // 'times' gives cumulative user+system time in seconds on some
  // implementations, or we can use 'cputime'. We use 'times' which is usually
  // accumulator. Note: We multiply seconds by CLK_TCK (usually 100) to fake
  // "Jiffies" for consistency.
  std::string cmd = "ps -eo pid,rss,times,etimes,comm --no-headers";

  std::string output = execute_ssh_command(cmd.c_str());
  std::stringstream ss(output);
  std::string line;

  while (std::getline(ss, line)) {
    std::stringstream lss(line);
    long pid;
    ProcessRawSnapshot snap;
    long cpu_seconds = 0;
    long elapsed_seconds = 0;

    if (lss >> pid >> snap.vmRssKb >> cpu_seconds) {
      // Fake the Jiffies: Seconds * 100.
      // This ensures the math in the Task (which divides by HZ) works for
      // both.
      snap.cumulative_cpu_time = cpu_seconds * 100;
      snap.start_time = elapsed_seconds;

      // Remainder of line is command
      std::getline(lss, snap.name);

      // Trim leading space from name
      size_t first = snap.name.find_first_not_of(" ");
      if (first != std::string::npos)
        snap.name = snap.name.substr(first);

      snapshots[pid] = snap;
    }
  }
  return snapshots;
}

// --- POLLING TASK LOGIC ---
ProcessPollingTask::ProcessPollingTask(DataStreamProvider &p, SystemMetrics &m,
                                       MetricsContext &context)
    : IPollingTask(p, m, context) {
  auto settings = context.settings;

  process_count = settings.process_count;
  ignore_list = settings.ignore_list;
  only_user_processes = settings.only_user_processes;

  // 1. CPU Configuration
  if (settings.enable_realtime_processinfo_cpu ||
      settings.enable_avg_processinfo_cpu) {
    bool need_real = settings.enable_realtime_processinfo_cpu;
    bool need_avg = settings.enable_avg_processinfo_cpu;
    DEBUG_PTR("Pipeline vector", output_pipeline);

    output_pipeline.emplace_back(
        [this, need_real, need_avg](std::vector<ProcessInfo> &data) {
          DEBUG_PTR("ProcessPollingTask lambda this", this);
          DEBUG_PTR("ProcessPollingTask lambda metrics", metrics);
          if (need_real) {
            // Sort by Realtime CPU
            this->populate_top_ps(data, this->metrics.top_processes_real_cpu,
                                  SortMode::CPU_REAL);

            if (need_avg) {
              // If we have both, usually we want the list to match the Realtime
              // list, just with Avg data filled in. So we copy.
              this->metrics.top_processes_avg_cpu =
                  this->metrics.top_processes_real_cpu;
            }
          } else {
            // Only Avg requested: Sort specifically by Lifetime Average
            this->populate_top_ps(data, this->metrics.top_processes_avg_cpu,
                                  SortMode::CPU_AVG);
          }
          SPDLOG_TRACE("CPU Lambda: Exiting scope.");
        });
  }

  // 2. Memory Configuration
  if (settings.enable_realtime_processinfo_mem ||
      settings.enable_avg_processinfo_mem) {
    bool need_real = settings.enable_realtime_processinfo_mem;
    bool need_avg = settings.enable_avg_processinfo_mem;

    output_pipeline.emplace_back(
        [this, need_real, need_avg](std::vector<ProcessInfo> &data) {
          DEBUG_PTR("ProcessPollingTask lambda this", this);
          DEBUG_PTR("ProcessPollingTask lambda metrics", metrics);
          // Always sort by Memory (RSS)
          this->populate_top_ps(data, this->metrics.top_processes_real_mem,
                                SortMode::MEM);

          if (need_avg) {
            this->metrics.top_processes_avg_mem =
                this->metrics.top_processes_real_mem;
          }

          // If real is disabled but avg is enabled, we still use the REAL list
          // because "Average Memory" isn't really a distinct thing (it's just
          // current usage).
          if (!need_real && need_avg) {
            SPDLOG_TRACE("pipeline (alt path)");
            this->metrics.top_processes_avg_mem =
                this->metrics.top_processes_real_mem;
          }
          SPDLOG_TRACE("Memory Lambda: Exiting scope.");
        });
  }
}
void ProcessPollingTask::take_initial_snapshot() {
  set_timestamp();
  prev_snapshots = read_data();
}
void ProcessPollingTask::take_new_snapshot() {
  set_delta_time();
  current_snapshots = read_data();
}

ProcessSnapshotMap ProcessPollingTask::read_data() {
  return provider.get_process_snapshots(only_user_processes);
}

void ProcessPollingTask::commit() {
  prev_snapshots = std::move(current_snapshots);
}

// Helper lambda for sorting and assigning top 10
void ProcessPollingTask::populate_top_ps(std::vector<ProcessInfo> &source,
                                         std::vector<ProcessInfo> &dest,
                                         SortMode mode) {
  SPDLOG_TRACE("  Sort: Start. Size: {}", source.size());
  dest.reserve(process_count);

  // Define the comparator based on the mode
  auto sorter = [mode](const ProcessInfo &a, const ProcessInfo &b) {
    switch (mode) {
    case SortMode::MEM:
      return a.vmRssKb > b.vmRssKb;
    case SortMode::CPU_REAL:
      return a.cpu_percent > b.cpu_percent;
    case SortMode::CPU_AVG:
      return a.cpu_avg_percent > b.cpu_avg_percent;
    }
    return false;
  };

  if (source.size() > process_count) {
    std::partial_sort(source.begin(), source.begin() + process_count,
                      source.end(), sorter);
    dest.assign(source.begin(), source.begin() + process_count);
  } else {
    std::sort(source.begin(), source.end(), sorter);
    dest = source;
  }
  SPDLOG_TRACE("  Sort: Finished.");
}
void ProcessPollingTask::calculate() {
  // 1. Clear all destination vectors
  metrics.top_processes_avg_mem.clear();
  metrics.top_processes_avg_cpu.clear();
  metrics.top_processes_real_mem.clear();
  metrics.top_processes_real_cpu.clear();

  DEBUG_PTR("Pipeline vector", output_pipeline);

  if (time_delta_seconds <= 0.0)
    return;

  static const long CLK_TCK = sysconf(_SC_CLK_TCK);
  double total_jiffies_available =
      static_cast<double>(CLK_TCK) * time_delta_seconds;

  long system_uptime_jiffies = get_system_uptime_jiffies();

  std::vector<ProcessInfo> all_procs;
  all_procs.reserve(current_snapshots.size());

  // 2. Calculate Real-Time Delta for ALL processes
  for (const auto &[pid, current_snap] : current_snapshots) {
    if (std::find(ignore_list.begin(), ignore_list.end(), current_snap.name) !=
        ignore_list.end()) {
      continue;
    }

    auto prev_it = prev_snapshots.find(pid);
    if (prev_it != prev_snapshots.end()) {
      const auto &prev_snap = prev_it->second;

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
      // A safe heuristic:
      double lifetime_usage = 0.0;

      // Local Logic: current_snap.start_time is Jiffies
      if (system_uptime_jiffies > 0 && current_snap.start_time > 100000) {
        double elapsed_jiffies =
            system_uptime_jiffies - current_snap.start_time;
        if (elapsed_jiffies > 0) {
          lifetime_usage =
              (current_snap.cumulative_cpu_time / elapsed_jiffies) * 100.0;
        }
      }
      // SSH Logic: current_snap.start_time is Elapsed Seconds (via etimes)
      else if (current_snap.start_time > 0) {
        // cumulative_cpu_time is (Seconds * 100), start_time is Seconds
        // Convert back to pure seconds for ratio
        double cpu_sec = current_snap.cumulative_cpu_time / 100.0;
        lifetime_usage = (cpu_sec / (double)current_snap.start_time) * 100.0;
      }

      info.cpu_avg_percent = std::min(lifetime_usage, 100.0);

      // --- Memory Calculation ---
      if (metrics.meminfo.total_kb > 0) {
        info.mem_percent =
            (static_cast<double>(info.vmRssKb) / metrics.meminfo.total_kb) *
            100.0;
      }

      all_procs.push_back(std::move(info));
    }
  }
  SPDLOG_TRACE("Starting pipeline execution. Steps: {}",
               output_pipeline.size());

  int step_index = 0;
  for (const auto &task : output_pipeline) {
    SPDLOG_TRACE(" [Step {}] Invoking task...", step_index);

    // EXECUTE
    task(all_procs);

    SPDLOG_TRACE(" [Step {}] Task finished.", step_index);
    step_index++;
  }

  audit_process_list(metrics.top_processes_avg_mem);
  audit_process_list(metrics.top_processes_avg_cpu);
  audit_process_list(metrics.top_processes_real_mem);
  audit_process_list(metrics.top_processes_real_cpu);

  SPDLOG_TRACE("Pipeline complete with IO/FD audit.");
}
void ProcessPollingTask::set_process_count(int count) { process_count = count; }
void ProcessPollingTask::audit_process_list(std::vector<ProcessInfo> &list) {
  for (auto &proc : list) {
    std::string pid_path = "/proc/" + std::to_string(proc.pid);

    // 1. Count Open File Descriptors
    try {
      auto fd_path = std::filesystem::path(pid_path) / "fd";
      if (std::filesystem::exists(fd_path)) {
        proc.open_fds =
            std::distance(std::filesystem::directory_iterator(fd_path),
                          std::filesystem::directory_iterator{});
      }
    } catch (...) {
      proc.open_fds = -1;
    }

    // 2. Read IO Stats
    std::ifstream io_file(pid_path + "/io");
    std::string label;
    while (io_file >> label) {
      if (label == "read_bytes:")
        io_file >> proc.io_read_bytes;
      else if (label == "write_bytes:")
        io_file >> proc.io_write_bytes;
      io_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
  }
}

bool Processes ::enable_processinfo() const {
  return enable_avg_cpu || enable_avg_mem || enable_realtime_cpu ||
         enable_realtime_mem;
}

std::string
LuaProcesses ::serialize(unsigned const int indentation_level) const {
  LuaConfigGenerator processes("processes", indentation_level);
  processes.lua_bool("enable_processinfo", enable_processinfo());
  processes.lua_bool("enable_avg_cpu", enable_avg_cpu);
  processes.lua_bool("enable_avg_mem", enable_avg_mem);
  processes.lua_bool("enable_realtime_cpu", enable_realtime_cpu);
  processes.lua_bool("enable_realtime_mem", enable_realtime_mem);
  processes.lua_uint("count", count);
  processes.lua_vector("ignore_list", ignore_list); // fixme
  return processes.str();
}

void LuaProcesses::deserialize(sol::table procs) {
  if (procs.valid()) {
    enable_avg_cpu =
        procs.get<sol::optional<bool>>("enable_avg_cpu").value_or(true);
    enable_avg_mem =
        procs.get<sol::optional<bool>>("enable_avg_mem").value_or(true);
    enable_realtime_cpu =
        procs.get<sol::optional<bool>>("enable_realtime_cpu").value_or(true);
    enable_realtime_mem =
        procs.get<sol::optional<bool>>("enable_realtime_mem").value_or(true);
    only_user_processes =
        procs.get<sol::optional<bool>>("only_user_processes").value_or(true);
    count = procs.get<sol::optional<long unsigned int>>(std::string("count"))
                .value_or(10);
    ignore_list =
        procs.get<sol::optional<std::vector<std::string>>>("ignore_list")
            .value_or(std::vector<std::string>{});
  }
}

}; // namespace telemetry
