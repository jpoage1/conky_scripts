#include "mount_info.h"

#include <limits.h>
#include <string.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>

std::string get_mount_point(const std::string& device_path) {
  std::ifstream mounts("/proc/mounts");
  return get_mount_point(mounts, device_path);
}

std::string get_mount_point(std::istream& mounts_stream,
                            const std::string& device_path) {
  std::string device, mount_point, rest;

  mounts_stream.seekg(0);
  mounts_stream.clear();

  // 1. Calculate the canonical path for the device we are looking for once.
  std::filesystem::path target_path_canonical;
  try {
    target_path_canonical = std::filesystem::canonical(device_path);
  } catch (const std::filesystem::filesystem_error& e) {
    // If the target device_path doesn't exist, we can't find it.
    return "";
  }

  while (mounts_stream >> device >> mount_point) {
    std::getline(mounts_stream, rest);

    // 2. Use the memory-safe std::filesystem::canonical for comparison.
    try {
      std::filesystem::path current_dev_canonical =
          std::filesystem::canonical(device);

      // Compare the canonical paths.
      if (current_dev_canonical == target_path_canonical) {
        return mount_point;
      }
    } catch (const std::filesystem::filesystem_error& e) {
      // Ignore errors for devices like "proc" or "sysfs" that may not resolve.
      continue;
    }
  }
  return "";
}
