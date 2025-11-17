#pragma once

#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "data.h"
#include "pcn.hpp"

struct DiskIO {
  std::string read_per_sec;
  std::string write_per_sec;
};

struct DiskUsage {
  uint64_t used_bytes = 0;
  uint64_t size_bytes = 0;
};

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

std::vector<DeviceInfo> collect_device_info(
    const std::vector<std::string>& device_paths);
std::vector<DeviceInfo> collect_device_info(
    DataStreamProvider& provider, const std::vector<std::string>& device_paths);

std::string get_mount_point(const std::string& device_path);
std::string get_mount_point(std::istream&, const std::string& device_path);

std::string get_read_bytes_per_sec(const std::string&);
std::string get_read_bytes_per_sec(std::istream&,
                                   const std::string& device_path);

std::string get_write_bytes_per_sec(const std::string&);
std::string get_write_bytes_per_sec(std::istream&,
                                    const std::string& device_path);

DiskIO get_disk_io_per_sec(const std::string&);
DiskIO get_disk_io_per_sec(std::istream&, const std::string&);

DiskUsage get_disk_usage(const std::string& mount_point);
