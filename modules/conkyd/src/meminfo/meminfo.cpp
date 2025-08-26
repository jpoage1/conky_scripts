#include "meminfo.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <string>

void get_mem_usage(std::istream &input_stream, long &used, long &total,
                   int &percent) {
  std::string label;
  long mem_total = 0, mem_free = 0, buffers = 0, cached = 0;
  while (input_stream >> label) {
    if (label == "MemTotal:")
      input_stream >> mem_total;
    else if (label == "MemFree:")
      input_stream >> mem_free;
    else if (label == "Buffers:")
      input_stream >> buffers;
    else if (label == "Cached:")
      input_stream >> cached;
    if (mem_total && mem_free && buffers && cached) break;
    input_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }
  total = mem_total;
  used = mem_total - (mem_free + buffers + cached);
  percent = total == 0 ? 0 : (100 * used / total);
}
