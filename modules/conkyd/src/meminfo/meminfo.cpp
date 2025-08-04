#include "meminfo.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <string>

void get_mem_usage(long &used, long &total, int &percent) {
  std::ifstream file("/proc/meminfo");
  std::string label;
  long mem_total = 0, mem_free = 0, buffers = 0, cached = 0;
  while (file >> label) {
    if (label == "MemTotal:")
      file >> mem_total;
    else if (label == "MemFree:")
      file >> mem_free;
    else if (label == "Buffers:")
      file >> buffers;
    else if (label == "Cached:")
      file >> cached;
    if (mem_total && mem_free && buffers && cached) break;
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }
  total = mem_total;
  used = mem_total - (mem_free + buffers + cached);
  percent = total == 0 ? 0 : (100 * used / total);
}
