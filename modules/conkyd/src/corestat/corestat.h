// corestat.h
#pragma once

#include <istream>
#include <string>
#include <vector>

struct CPUCore {
  unsigned long long idle_time;
  unsigned long long total_time;
};

std::vector<CPUCore> read_cpu_times(std::istream &);
std::vector<float> get_cpu_usages(std::istream &);
std::string format_cpu_times(const CPUCore &, size_t);
