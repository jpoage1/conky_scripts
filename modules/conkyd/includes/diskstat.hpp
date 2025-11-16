// includes/diskstat.hpp
#pragma once
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
using DiskStatConfig = std::set<DiskStatSettings>;

int read_device_paths(const std::string& path, std::vector<std::string>&);

void diskstat(const std::string& config_file);

int diskstat(DataStreamProvider& provider, const std::string& config_file);
