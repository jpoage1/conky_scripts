// processinfo.hpp
#pragma once

#include <algorithm>
#include <cmath>  // For std::round

#include "data_local.hpp"
#include "data_ssh.hpp"
#include "pcn.hpp"

// Data structure to hold information about a single process
struct ProcessInfo {
  int pid = 0;
  long vmRssKb = 0;  // Resident Set Size in KiB
  double cpu_percent = 0.0;
  double mem_percent = 0.0;
  std::string name;
};
struct CpuState {
  long jiffies;
  std::chrono::steady_clock::time_point timestamp;
};

void get_top_processes(std::istream& stream,
                       std::vector<ProcessInfo>& output_list,
                       long mem_total_kb);

// Assumed helper: Reads a single value from a file (e.g., VmRSS)
long read_proc_status_field(const std::string& pid_dir,
                            const std::string& field_name);

// Assumed helper: Extracts CPU usage from /proc/[pid]/stat
double get_process_cpu_usage(long pid);

void get_top_processes(std::vector<ProcessInfo>& output_list, long mem_total_kb,
                       bool sort_by_mem);
