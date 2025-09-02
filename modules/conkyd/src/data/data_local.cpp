#include "data_local.h"

LocalDataStreams get_local_file_streams() {
  LocalDataStreams streams;
  streams.cpuinfo.open("/proc/cpuinfo");
  streams.meminfo.open("/proc/meminfo");
  streams.uptime.open("/proc/uptime");
  streams.stat.open("/proc/stat");
  streams.mounts.open("/proc/mounts");
  streams.diskstats.open("/proc/diskstats");
  return streams;
}

LocalDataStreams get_local_file_streams();

uint64_t LocalDataStreams::get_used_space_bytes(
    const std::string &mount_point) {
  struct statvfs stat;
  if (mount_point.empty() || statvfs(mount_point.c_str(), &stat) != 0) return 0;
  return (stat.f_blocks - stat.f_bfree) * stat.f_frsize;
}

uint64_t LocalDataStreams::get_disk_size_bytes(const std::string &mount_point) {
  struct statvfs stat;
  if (mount_point.empty() || statvfs(mount_point.c_str(), &stat) != 0) return 0;
  return stat.f_blocks * stat.f_frsize;
}
