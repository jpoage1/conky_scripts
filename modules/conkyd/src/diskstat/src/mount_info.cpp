#include "mount_info.h"

#include <limits.h>
#include <string.h>
#include <unistd.h>

#include <fstream>

std::string get_mount_point(const std::string &device_path) {
  std::ifstream mounts("/proc/mounts");
  return get_mount_point(mounts, device_path);
}

std::string get_mount_point(std::istream &mounts_stream,
                            const std::string &device_path) {
  std::string device, mount_point, rest;

  mounts_stream.seekg(0);
  mounts_stream.clear();

  while (mounts_stream >> device >> mount_point) {
    std::getline(mounts_stream, rest);
    // char dev_real[PATH_MAX], path_real[PATH_MAX];
    // if (realpath(device.c_str(), dev_real) &&
    //     realpath(device_path.c_str(), path_real)) {
    //   if (strcmp(dev_real, path_real) == 0) return mount_point;
    // }
    if (device == device_path) {
      return mount_point;
    }
  }
  return "";
}
