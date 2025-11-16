// meminfo.cpp
#include "meminfo.h"

#include "data_local.h"
#include "data_ssh.h"

std::istream& LocalDataStreams::get_meminfo_stream() {
  return create_stream_from_file(meminfo, "/proc/meminfo");
}

std::istream& ProcDataStreams::get_meminfo_stream() {
  return create_stream_from_command(meminfo, "cat /proc/meminfo");
}

void get_mem_usage(std::istream& input_stream, long& used, long& total,
                   int& percent) {
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

void get_swap_usage(std::istream& input_stream, long& used, long& total,
                    int& percent) {
  std::string label;
  long swap_total = 0, swap_free = 0;
  while (input_stream >> label) {
    if (label == "SwapTotal:")
      input_stream >> swap_total;
    else if (label == "SwapFree:")
      input_stream >> swap_free;
    if (swap_total && swap_free) break;
    input_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }
  total = swap_total;
  used = swap_total - swap_free;
  percent = total == 0 ? 0 : (100 * used / total);
}
