#include "device_info.h"
#include "disk_io.h"
#include "mount_info.h"
#include <sys/statvfs.h>

std::vector<DeviceInfo>
collect_device_info(const std::vector<std::string> &device_paths) {
  std::vector<DeviceInfo> data;
  for (const auto &device_path : device_paths) {
    DeviceInfo info;
    info.device_path = device_path;
    info.mount_point = get_mount_point(device_path);
    info.used_bytes = get_used_space_bytes(info.mount_point);
    info.size_bytes = get_disk_size_bytes(info.mount_point);
    info.read_bytes_per_sec = get_read_bytes_per_sec(device_path);
    info.write_bytes_per_sec = get_write_bytes_per_sec(device_path);
    data.push_back(info);
  }
  return data;
}

uint64_t get_used_space_bytes(const std::string &mount_point) {
  struct statvfs stat;
  if (mount_point.empty() || statvfs(mount_point.c_str(), &stat) != 0)
    return 0;
  return (stat.f_blocks - stat.f_bfree) * stat.f_frsize;
}

uint64_t get_disk_size_bytes(const std::string &mount_point) {
  struct statvfs stat;
  if (mount_point.empty() || statvfs(mount_point.c_str(), &stat) != 0)
    return 0;
  return stat.f_blocks * stat.f_frsize;
}
