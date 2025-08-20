#pragma once

#include <string>

#include "types.h"

struct DiskIO {
  std::string read_per_sec;
  std::string write_per_sec;
};

struct ConkyDiskIO {
  ColoredString read_per_sec;
  ColoredString write_per_sec;
};

std::string get_read_bytes_per_sec(const std::string &);
std::string get_write_bytes_per_sec(const std::string &);

DiskIO get_disk_io_per_sec(const std::string &);
ConkyDiskIO conky_get_disk_io_per_sec(const std::string &);
