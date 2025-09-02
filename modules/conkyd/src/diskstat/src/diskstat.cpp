#include "diskstat.h"

#include <filesystem>
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

int read_device_paths(const std::string &file_path,
                      std::vector<std::string> &lines) {
  std::ifstream file(file_path);
  std::string line;

  namespace fs = std::filesystem;

  // Check if the file exists, is a regular file, and is readable
  if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) {
    std::cerr << "Unable to load file: " + file_path << std::endl;
    return 1;
  }

  while (std::getline(file, line)) {
    if (!line.empty()) lines.push_back(line);
  }
  return 0;
}

int diskstat(const std::string &config_file) {
  std::vector<std::string> device_paths;
  if (read_device_paths(config_file, device_paths) == 1) {
    std::cout << "${color red}Unable to load file: " + config_file + "${color}"
              << std::endl;
    return 1;
  }
  auto devices = collect_device_info(device_paths);
  size_t column_count = sizeof(columns) / sizeof(columns[0]);
  std::cout.setf(std::ios::unitbuf);

  print_column_headers(columns, column_count);
  print_rows(devices, column_count);
  return 0;
}

int diskstat(DataStreamProvider &provider, const std::string &config_file) {
  std::vector<std::string> device_paths;
  if (read_device_paths(config_file, device_paths) == 1) {
    std::cout << "${color red}Unable to load file: " + config_file + "${color}"
              << std::endl;
    return 1;
  }
  auto devices = collect_device_info(provider, device_paths);
  size_t column_count = sizeof(columns) / sizeof(columns[0]);
  std::cout.setf(std::ios::unitbuf);

  print_column_headers(columns, column_count);
  print_rows(devices, column_count);
  return 0;
}
