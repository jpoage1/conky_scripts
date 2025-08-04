#include "swapinfo.h"

#include <fstream>
#include <limits>
#include <string>

void get_swap_usage(long &used, long &total, int &percent) {
  std::ifstream file("/proc/meminfo");
  std::string label;
  long swap_total = 0, swap_free = 0;
  while (file >> label) {
    if (label == "SwapTotal:")
      file >> swap_total;
    else if (label == "SwapFree:")
      file >> swap_free;
    if (swap_total && swap_free) break;
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }
  total = swap_total;
  used = swap_total - swap_free;
  percent = total == 0 ? 0 : (100 * used / total);
}
