// processinfo.hpp
#ifndef PROCESSINFO_HPP
#define PROCESSINFO_HPP

#include <algorithm>
#include <cmath> // For std::round

#include "pcn.hpp"
namespace telemetry {

class LuaConfigGenerator;
// Data structure to hold information about a single process
struct ProcessInfo {
  int pid = 0;
  long vmRssKb = 0; // Resident Set Size in KiB
  double cpu_percent = 0.0;
  double mem_percent = 0.0;
  double cpu_avg_percent = 0.0;
  uint64_t io_read_bytes = 0;
  uint64_t io_write_bytes = 0;
  int open_fds = 0;

  std::string name;
};
struct CpuState {
  long jiffies;
  std::chrono::steady_clock::time_point timestamp;
};
enum class SortMode { MEM, CPU_REAL, CPU_AVG };

struct Processes {
  bool enable_avg_cpu = true;
  bool enable_avg_mem = true;
  bool enable_realtime_cpu = true;
  bool enable_realtime_mem = true;
  long unsigned int count = true;
  std::vector<std::string> ignore_list;
  bool only_user_processes = false;
  bool enable_processinfo() const;
}; // End Processes struct

// Processes
struct LuaProcesses : public Processes {
  std::string serialize(unsigned const int indentation_level = 0) const;

  void deserialize(sol::table processes);
}; // End Processes struct

}; // namespace telemetry
#endif
