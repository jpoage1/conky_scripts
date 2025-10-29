// swapinfo.cpp
#include "swapinfo.h"

#include <fstream>
#include <limits>
#include <string>

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
