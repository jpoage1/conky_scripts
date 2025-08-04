// corestat.h
#pragma once

#include <vector>

struct CPUCore {
  unsigned long long idle_time;
  unsigned long long total_time;
};

std::vector<CPUCore> read_cpu_times();
std::vector<float> get_cpu_usages();
std::string format_cpu_times(const CpuTimes&, size_t);
