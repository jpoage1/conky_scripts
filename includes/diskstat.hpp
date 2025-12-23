// includes/diskstat.hpp
#ifndef DISKSTAT_HPP
#define DISKSTAT_HPP
#include <unistd.h>

#include <climits>
#include <cstring>

#include "pcn.hpp"

class DataStreamProvider;

enum DiskStatSettings {
  Loopback,
  MapperDevices,
  Partitions,
};

struct DiskIoSnapshot {
  uint64_t bytes_read = 0;
  uint64_t bytes_written = 0;
};

struct DiskIoStats {
  uint64_t read_bytes_per_sec = 0;
  uint64_t write_bytes_per_sec = 0;
};

struct HdIoStats {
  std::string device_name;
  uint64_t read_bytes_per_sec = 0;
  uint64_t write_bytes_per_sec = 0;
};

struct DiskUsage {
  uint64_t used_bytes = 0;
  uint64_t size_bytes = 0;
};
struct DeviceInfo {
  std::string device_path;
  std::string mount_point;
  DiskUsage usage;
  DiskIoStats io;
};
using DeviceMap = std::map<std::string, DeviceInfo>;

using DiskStatConfig = std::set<DiskStatSettings>;

int read_device_paths(const std::string& path, std::vector<std::string>&);

void diskstat(const std::string& config_file);

int diskstat(DataStreamProvider& provider, const std::string& config_file);

std::string get_mount_point(const std::string& device_path);
std::string get_mount_point(std::istream&, const std::string& device_path);
#endif
