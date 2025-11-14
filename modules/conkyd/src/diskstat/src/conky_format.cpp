// conky_format.cpp
#include "size_format.h"


#include <cstddef>
#include "colors.h"
#include "types.h"
#include "conky_format.h"

std::tuple<std::string, std::function<FuncType>> conky_columns[] = {
    {"Device",
     [](const DeviceInfo& d) { return conky_color(d.device_path, palerblue); }},
    {"Mount Point",
     [](const DeviceInfo& d) { return conky_color(d.mount_point, lightgrey); }},
    {"Used", [](const DeviceInfo& d) { return format_size(d.used_bytes); }},
    {"Free",
     [](const DeviceInfo& d) {
       return format_size(d.size_bytes - d.used_bytes);
     }},
    {"Size", [](const DeviceInfo& d) { return format_size(d.size_bytes); }},
    {"Used%",
     [](const DeviceInfo& d) {
       if (d.size_bytes == 0) return conky_color("N/A", gray);
       uint64_t percent = (d.used_bytes * 100) / d.size_bytes;
       std::string color = percent >= 90 ? red : percent >= 75 ? yellow : green;
       return conky_color(std::to_string(percent) + "%", color);
     }},
    {"Read/s",
     [](const DeviceInfo& d) {
       return conky_color(
           d.read_bytes_per_sec.empty() ? "N/A" : d.read_bytes_per_sec,
           d.read_bytes_per_sec.empty() ? gray : blue);
     }},
    {"Write/s",
     [](const DeviceInfo& d) {
       return conky_color(
           d.write_bytes_per_sec.empty() ? "N/A" : d.write_bytes_per_sec,
           d.write_bytes_per_sec.empty() ? gray : blue);
     }},
};

extern const size_t CONKY_COLUMNS_COUNT = sizeof(conky_columns) / sizeof(conky_columns[0]);

ColoredString conky_color(const std::string &text,
                          const std::string &color = lightgrey) {
  return ColoredString{text, color};
}
