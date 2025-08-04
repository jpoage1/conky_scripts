#include "colors.h"
#include "conky_format.h"
#include "size_format.h"

ColoredString format_size(uint64_t bytes) {
  constexpr uint64_t KB = 1024;
  constexpr uint64_t MB = KB * 1024;
  constexpr uint64_t GB = MB * 1024;

  if (bytes >= GB)
    return conky_color(std::to_string(bytes / GB) + "G", "00ff00");
  else if (bytes >= MB)
    return conky_color(std::to_string(bytes / MB) + "M", "ffaa00");
  else if (bytes >= KB)
    return conky_color(std::to_string(bytes / KB) + "K", red);
  else if (bytes > 0)
    return conky_color(std::to_string(bytes) + "B", red);
  else
    return conky_color("N/A", gray);
}
