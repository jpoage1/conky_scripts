#include "disk_io.h"
#include "size_format.h"
#include "types.h"
#include <cctype>
#include <chrono>
#include <fstream>
#include <sstream>
#include <unordered_map>

static std::unordered_map<std::string, DeviceStats> stats_map;

static uint64_t current_time_in_ms() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
      .count();
}

std::string get_read_bytes_per_sec(const std::string &device_path) {
  if (device_path.empty())
    return "";

  std::string device_name =
      device_path.substr(device_path.find_last_of('/') + 1);
  while (!device_name.empty() && isdigit(device_name.back()))
    device_name.pop_back();

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
  if (device_path.empty())
    return "";

  std::string device_name =
      device_path.substr(device_path.find_last_of('/') + 1);
  while (!device_name.empty() && isdigit(device_name.back()))
    device_name.pop_back();

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
