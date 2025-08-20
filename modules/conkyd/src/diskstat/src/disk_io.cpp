#include "disk_io.h"

#include <cctype>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "colors.h"
#include "conky_format.h"
#include "size_format.h"
#include "types.h"

static std::unordered_map<std::string, DeviceStats> stats_map;

static uint64_t current_time_in_ms() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
      .count();
}

std::string get_read_bytes_per_sec(const std::string &device_path) {
  if (device_path.empty()) {
    return "";
  }

  std::string device_name =
      device_path.substr(device_path.find_last_of('/') + 1);
  while (!device_name.empty() && isdigit(device_name.back())) {
    device_name.pop_back();
  }

  uint64_t current_bytes_read = 0;
  std::ifstream diskstats("/proc/diskstats");
  std::string line;
  while (std::getline(diskstats, line)) {
    std::istringstream iss(line);
    std::string dev;
    uint64_t sectors = 0;
    int field = 0;
    while (iss >> dev) {
      if (++field == 3)
        dev = dev;
      else if (field == 6)
        sectors = std::stoull(dev);
    }
    if (dev == device_name) {
      current_bytes_read = sectors * 512;
      break;
    }
  }

  uint64_t now = current_time_in_ms();
  auto &stat = stats_map[device_name];
  uint64_t delta_bytes = 0;
  uint64_t delta_ms = 1;

  if (stat.prev_timestamp_ms && now > stat.prev_timestamp_ms) {
    delta_bytes = current_bytes_read - stat.prev_bytes_read;
    delta_ms = now - stat.prev_timestamp_ms;
  }

  stat.prev_bytes_read = current_bytes_read;
  stat.prev_timestamp_ms = now;
  return format_size((delta_bytes * 1000) / delta_ms).text;
}

std::string get_write_bytes_per_sec(const std::string &device_path) {
  if (device_path.empty()) {
    return "";
  }

  std::string device_name =
      device_path.substr(device_path.find_last_of('/') + 1);
  while (!device_name.empty() && isdigit(device_name.back())) {
    device_name.pop_back();
  }

  uint64_t current_bytes_written = 0;
  std::ifstream diskstats("/proc/diskstats");
  std::string line;
  while (std::getline(diskstats, line)) {
    std::istringstream iss(line);
    std::string dev;
    uint64_t sectors = 0;
    int field = 0;
    while (iss >> dev) {
      if (++field == 3)
        dev = dev;
      else if (field == 10)
        sectors = std::stoull(dev);
    }
    if (dev == device_name) {
      current_bytes_written = sectors * 512;
      break;
    }
  }

  uint64_t now = current_time_in_ms();
  auto &stat = stats_map[device_name];
  uint64_t delta_bytes = 0;
  uint64_t delta_ms = 1;

  if (stat.prev_timestamp_ms && now > stat.prev_timestamp_ms) {
    delta_bytes = current_bytes_written - stat.prev_bytes_written;
    delta_ms = now - stat.prev_timestamp_ms;
  }

  stat.prev_bytes_written = current_bytes_written;
  stat.prev_timestamp_ms = now;

  return format_size((delta_bytes * 1000) / delta_ms).text;
}

ConkyDiskIO conky_get_disk_io_per_sec(const std::string &device_path) {
  return {conky_color("${diskio_read " + device_path + "}", "ffaa00"),
          conky_color("${diskio_write " + device_path + "}", "ffaa00")};
}

DiskIO get_disk_io_per_sec(const std::string &device_path) {
  if (device_path.empty()) return {};

  std::string device_name =
      device_path.substr(device_path.find_last_of('/') + 1);
  while (!device_name.empty() && isdigit(device_name.back()))
    device_name.pop_back();

  uint64_t current_bytes_read = 0;
  uint64_t current_bytes_written = 0;

  std::ifstream diskstats("/proc/diskstats");
  std::string line;
  while (std::getline(diskstats, line)) {
    std::istringstream iss(line);
    int major, minor;
    std::string dev;
    uint64_t reads_completed, reads_merged, sectors_read, time_reading;
    uint64_t writes_completed, writes_merged, sectors_written, time_writing;
    iss >> major >> minor >> dev >> reads_completed >> reads_merged >>
        sectors_read >> time_reading >> writes_completed >> writes_merged >>
        sectors_written >> time_writing;
    if (dev == device_name) {
      current_bytes_read = sectors_read * 512;
      current_bytes_written = sectors_written * 512;
      break;
    }
  }

  uint64_t now = current_time_in_ms();
  auto &stat = stats_map[device_name];
  uint64_t delta_read = 0, delta_write = 0;
  uint64_t delta_ms = 1;

  if (stat.prev_timestamp_ms && now > stat.prev_timestamp_ms) {
    delta_read = current_bytes_read - stat.prev_bytes_read;
    delta_write = current_bytes_written - stat.prev_bytes_written;
    delta_ms = now - stat.prev_timestamp_ms;
  }

  stat.prev_bytes_read = current_bytes_read;
  stat.prev_bytes_written = current_bytes_written;
  stat.prev_timestamp_ms = now;

  return {format_size((delta_read * 1000) / delta_ms).text,
          format_size((delta_write * 1000) / delta_ms).text};
}
