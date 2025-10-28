
#include "data_local.h"

#include <filesystem>
#include <iostream>
#include <optional>

void rewind(std::ifstream& stream) {
  // Debug: Check if stream is in a bad state before attempting reset
  if (stream.fail() || stream.bad()) {
    std::cerr << "DEBUG: Stream was in fail/bad state before rewind."
              << std::endl;
  }

  stream.clear();
  stream.seekg(0, std::ios::beg);

  // Debug: Confirm stream is usable after reset
  if (stream.fail() || stream.bad()) {
    std::cerr << "DEBUG: Stream is still in fail/bad state after rewind. FATAL."
              << std::endl;
  }
}

LocalDataStreams get_local_file_streams() {
  LocalDataStreams streams;
  streams.cpuinfo.open("/proc/cpuinfo");
  streams.meminfo.open("/proc/meminfo");
  streams.uptime.open("/proc/uptime");
  streams.stat.open("/proc/stat");
  streams.mounts.open("/proc/mounts");
  streams.diskstats.open("/proc/diskstats");
  streams.loadavg.open("/proc/loadavg");
  streams.net_dev.open("/proc/net/dev");
  return streams;
}

uint64_t LocalDataStreams::get_used_space_bytes(
    const std::string& mount_point) {
  struct statvfs stat;
  if (mount_point.empty() || statvfs(mount_point.c_str(), &stat) != 0) return 0;
  return (stat.f_blocks - stat.f_bfree) * stat.f_frsize;
}

uint64_t LocalDataStreams::get_disk_size_bytes(const std::string& mount_point) {
  struct statvfs stat;
  if (mount_point.empty() || statvfs(mount_point.c_str(), &stat) != 0) return 0;
  return stat.f_blocks * stat.f_frsize;
}

std::optional<std::string> read_sysfs_file(const std::filesystem::path& path) {
  if (!std::filesystem::exists(path)) {
    return std::nullopt;
  }
  std::ifstream file(path);
  if (!file.is_open()) {
    return std::nullopt;
  }
  std::string line;
  if (std::getline(file, line)) {
    return line;
  }
  return std::nullopt;
}

double LocalDataStreams::get_cpu_temperature() {
  const std::string hwmon_base = "/sys/class/hwmon";
  std::optional<double> fallback_temp;  // Fallback to first core/sensor

  try {
    for (const auto& hwmon_dir :
         std::filesystem::directory_iterator(hwmon_base)) {
      if (!hwmon_dir.is_directory()) continue;

      auto name = read_sysfs_file(hwmon_dir.path() / "name");
      if (!name || (name.value() != "coretemp" && name.value() != "k10temp" &&
                    name.value() != "zenpower")) {
        continue;  // Not a known CPU sensor
      }

      for (const auto& file :
           std::filesystem::directory_iterator(hwmon_dir.path())) {
        std::string filename = file.path().filename().string();

        if (filename.rfind("temp", 0) == 0 &&
            filename.rfind("_input") != std::string::npos) {
          std::string label_filename = filename;
          label_filename.replace(label_filename.rfind("_input"), 6, "_label");

          auto label = read_sysfs_file(hwmon_dir.path() / label_filename);
          auto temp_str = read_sysfs_file(file.path());

          if (!temp_str) continue;

          double temp_celsius = std::stod(temp_str.value()) / 1000.0;

          if (label) {
            if (label.value() == "Tdie" || label.value() == "Package id 0") {
              return temp_celsius;  // Found package temp
            }
          }
          if (!fallback_temp) {
            fallback_temp = temp_celsius;
          }
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error reading temperature: " << e.what() << std::endl;
    return -1.0;
  }

  if (fallback_temp) {
    return fallback_temp.value();
  }

  return -1.0;  // Not found
}
