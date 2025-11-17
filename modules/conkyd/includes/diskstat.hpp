// includes/diskstat.hpp
#pragma once
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "pcn.hpp"
struct DataStreamProvider;
struct DiskIoSnapshot {
  uint64_t bytes_read = 0;
  uint64_t bytes_written = 0;
};

struct DiskIoStats {
  std::string device_name;
  uint64_t read_bytes_per_sec = 0;
  uint64_t write_bytes_per_sec = 0;
};

enum DiskStatSettings {
  Loopback,
  MapperDevices,
  Partitions,
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

using DiskStatConfig = std::set<DiskStatSettings>;

int read_device_paths(const std::string& path, std::vector<std::string>&);

void diskstat(const std::string& config_file);

int diskstat(DataStreamProvider& provider, const std::string& config_file);

std::string get_mount_point(const std::string& device_path);
std::string get_mount_point(std::istream&, const std::string& device_path);
