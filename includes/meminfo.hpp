// meminfo.hpp
#ifndef MEMINFO_HPP
#define MEMINFO_HPP

#include "pcn.hpp"

struct MemInfo {
  long used_kb;
  long total_kb;
  int percent;
};
void get_mem_usage(std::istream& input_stream, MemInfo& meminfo,
                   MemInfo& swapinfo);
#endif
