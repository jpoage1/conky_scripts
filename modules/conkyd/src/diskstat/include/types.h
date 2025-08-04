#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

struct DeviceInfo {
  std::string device_path;
  std::string mount_point;
  uint64_t used_bytes = 0;
  uint64_t size_bytes = 0;
  std::string used_space;
  std::string size;
  std::string used_space_percent;
  std::string read_bytes_per_sec;
  std::string write_bytes_per_sec;
};

struct DeviceStats {
  uint64_t prev_bytes_read = 0;
  uint64_t prev_bytes_written = 0;
  uint64_t prev_timestamp_ms = 0;
};

struct ColoredString {
  std::string text;
  std::string color;
  std::string formatted() const {
    return "${color " + color + "}" + text + "${color}";
  }
  ColoredString(const std::string& t, const std::string& c)
      : text(t), color(c) {}
};

using FuncType = ColoredString(const DeviceInfo&);
