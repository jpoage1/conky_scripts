// conky_format.cpp
#include "conky_format.hpp"

#include "colors.hpp"
#include "filesystems.hpp"
#include "format.hpp"

std::tuple<std::string, std::function<FuncType>> conky_columns[] = {
    {"Device",
     [](const DeviceInfo& d) {
       return conky_color(d.device_path, colors::palerblue);
     }},
    {"Mount Point",
     [](const DeviceInfo& d) {
       return conky_color(d.mount_point, colors::lightgrey);
     }},
    {"Used",
     [](const DeviceInfo& d) { return format_size(d.usage.used_bytes); }},
    {"Free",
     [](const DeviceInfo& d) {
       return format_size(d.usage.size_bytes - d.usage.used_bytes);
     }},
    {"Size",
     [](const DeviceInfo& d) { return format_size(d.usage.size_bytes); }},
    {"Used%",
     [](const DeviceInfo& d) {
       if (d.usage.size_bytes == 0) return conky_color("N/A", colors::gray);
       uint64_t percent = (d.usage.used_bytes * 100) / d.usage.size_bytes;
       std::string color = percent >= 90   ? colors::red
                           : percent >= 75 ? colors::yellow
                                           : colors::green;
       return conky_color(std::to_string(percent) + "%", color);
     }},
    // {"Read/s",
    //  [](const DeviceInfo& d) {
    //    return conky_color(
    //        d.read_bytes_per_sec.empty() ? "N/A" : d.read_bytes_per_sec,
    //        d.read_bytes_per_sec.empty() ? gray : blue);
    //  }},
    // {"Write/s",
    //  [](const DeviceInfo& d) {
    //    return conky_color(
    //        d.write_bytes_per_sec.empty() ? "N/A" : d.write_bytes_per_sec,
    //        d.write_bytes_per_sec.empty() ? gray : blue);
    //  }},
};

extern const size_t CONKY_COLUMNS_COUNT =
    sizeof(conky_columns) / sizeof(conky_columns[0]);

ColoredString conky_color(const std::string& text,
                          const std::string& color = colors::lightgrey) {
  return ColoredString{text, color};
}
ColoredString format_size(uint64_t bytes) {
  constexpr uint64_t KB = 1024;
  constexpr uint64_t MB = KB * 1024;
  constexpr uint64_t GB = MB * 1024;

  if (bytes >= GB)
    return conky_color(std::to_string(bytes / GB) + "G", "00ff00");
  else if (bytes >= MB)
    return conky_color(std::to_string(bytes / MB) + "M", "ffaa00");
  else if (bytes >= KB)
    return conky_color(std::to_string(bytes / KB) + "K", colors::red);
  else if (bytes > 0)
    return conky_color(std::to_string(bytes) + "B", colors::red);
  else  // bytes <= 0
    return conky_color("0", colors::gray);
}
