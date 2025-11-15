// includes/diskstat.hpp
#pragma once
#include <cstdint>
#include <istream>
#include <map>
#include <string>

struct DiskIoSnapshot {
  uint64_t bytes_read = 0;
  uint64_t bytes_written = 0;
};

struct DiskIoStats {
  std::string device_name;
  uint64_t read_bytes_per_sec = 0;
  uint64_t write_bytes_per_sec = 0;
};
