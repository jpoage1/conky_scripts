#include "diskstat.h"

#include <fstream>
#include <iostream>
#include <vector>

std::tuple<std::string, std::function<FuncType>> columns[] = {
    {"Device",
     [](const DeviceInfo &d) { return conky_color(d.device_path, palerblue); }},
    {"Mount Point",
     [](const DeviceInfo &d) { return conky_color(d.mount_point, lightgrey); }},
    {"Used", [](const DeviceInfo &d) { return format_size(d.used_bytes); }},
    {"Free",
     [](const DeviceInfo &d) {
       return format_size(d.size_bytes - d.used_bytes);
     }},
    {"Size", [](const DeviceInfo &d) { return format_size(d.size_bytes); }},
    {"Used%",
     [](const DeviceInfo &d) {
       if (d.size_bytes == 0) return conky_color("N/A", gray);
       uint64_t percent = (d.used_bytes * 100) / d.size_bytes;
       std::string color = percent >= 90 ? red : percent >= 75 ? yellow : green;
       return conky_color(std::to_string(percent) + "%", color);
     }},
    {"Read/s",
     [](const DeviceInfo &d) {
       return conky_color(
           d.read_bytes_per_sec.empty() ? "N/A" : d.read_bytes_per_sec,
           d.read_bytes_per_sec.empty() ? gray : blue);
     }},
    {"Write/s",
     [](const DeviceInfo &d) {
       return conky_color(
           d.write_bytes_per_sec.empty() ? "N/A" : d.write_bytes_per_sec,
           d.write_bytes_per_sec.empty() ? gray : blue);
     }},
};

static std::vector<std::string> read_device_paths(const std::string &path) {
  std::ifstream file(path);
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty()) lines.push_back(line);
  }
  return lines;
}

int diskstat(const std::string &config_file) {
  auto device_paths = read_device_paths(config_file);
  auto devices = collect_device_info(device_paths);
  size_t column_count = sizeof(columns) / sizeof(columns[0]);
  std::cout.setf(std::ios::unitbuf);

  print_column_headers(columns, column_count);
  print_rows(devices, column_count);
  return 0;
}
