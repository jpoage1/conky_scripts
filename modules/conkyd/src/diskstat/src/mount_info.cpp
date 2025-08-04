#include "mount_info.h"
#include <fstream>
#include <limits.h>
#include <string.h>
#include <unistd.h>

std::string get_mount_point(const std::string &device_path) {
  std::ifstream mounts("/proc/mounts");
  std::string device, mount_point, rest;

  while (mounts >> device >> mount_point) {
    std::getline(mounts, rest);
    char dev_real[PATH_MAX], path_real[PATH_MAX];
    if (realpath(device.c_str(), dev_real) &&
        realpath(device_path.c_str(), path_real)) {
      if (strcmp(dev_real, path_real) == 0)
        return mount_point;
    }
  }
  return "";
}
