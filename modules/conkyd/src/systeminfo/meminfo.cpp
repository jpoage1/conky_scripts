// meminfo.cpp
#include "meminfo.hpp"

#include "data_local.hpp"
#include "data_ssh.hpp"

std::istream& LocalDataStreams::get_meminfo_stream() {
  return create_stream_from_file(meminfo, "/proc/meminfo");
}

std::istream& ProcDataStreams::get_meminfo_stream() {
  return create_stream_from_command(meminfo, "cat /proc/meminfo");
}

void get_mem_usage(std::istream& input_stream, MemInfo& meminfo,
                   MemInfo& swapinfo) {
  std::string label;
  long mem_total = -1, mem_available = -1, swap_total = -1, swap_free = -1;
  while (input_stream >> label) {
    if (label == "MemTotal:")
      input_stream >> mem_total;
    else if (label == "MemAvailable:")
      input_stream >> mem_available;
    else if (label == "SwapTotal:")
      input_stream >> swap_total;
    else if (label == "SwapFree:")
      input_stream >> swap_free;

    if (mem_total != -1 && mem_available != -1 && swap_total != -1 &&
        swap_free != -1)
      break;
    input_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }
  meminfo.total_kb = mem_total;
  meminfo.used_kb = mem_total - mem_available;
  meminfo.percent =
      meminfo.total_kb == 0 ? 0 : (100 * meminfo.used_kb / meminfo.total_kb);

  swapinfo.total_kb = swap_total;
  swapinfo.used_kb = swap_total - swap_free;
  swapinfo.percent = (swapinfo.total_kb == 0)
                         ? 0
                         : (100 * swapinfo.used_kb / swapinfo.total_kb);
}
