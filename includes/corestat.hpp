// corestat.hpp
#ifndef CORESTAT_HPP
#define CORESTAT_HPP

#include "pcn.hpp"

struct CPUCore {
  unsigned long long idle_time;
  unsigned long long total_time;
};

struct CpuSnapshot {
  unsigned long long user = 0;
  unsigned long long nice = 0;
  unsigned long long system = 0;
  unsigned long long idle = 0;
  unsigned long long iowait = 0;
  unsigned long long irq = 0;
  unsigned long long softirq = 0;
  unsigned long long steal = 0;

  unsigned long long get_total_time() const {
    return user + nice + system + idle + iowait + irq + softirq + steal;
  }
};

struct CoreStats {
  size_t core_id = 0;  // 0 will be aggregate, 1 will be Core 0, etc.
  float user_percent = 0.0f;
  float nice_percent = 0.0f;
  float system_percent = 0.0f;
  float iowait_percent = 0.0f;
  float idle_percent = 0.0f;
  float total_usage_percent = 0.0f;  // user + nice + system
};

std::vector<CPUCore> read_cpu_times(std::istream&);
std::string format_cpu_times(const CPUCore&, size_t);
#endif
