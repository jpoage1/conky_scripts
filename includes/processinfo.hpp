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
  double cpu_avg_percent = 0.0;
  std::string name;
};
struct CpuState {
  long jiffies;
  std::chrono::steady_clock::time_point timestamp;
};
enum class SortMode { MEM, CPU_REAL, CPU_AVG };
