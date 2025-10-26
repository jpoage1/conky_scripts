#include <sys/statvfs.h>

#include "data.h"

struct LocalDataStreams : public DataStreamProvider {
  std::ifstream cpuinfo;
  std::ifstream meminfo;
  std::ifstream uptime;
  std::ifstream stat;
  std::ifstream mounts;
  std::ifstream diskstats;

  std::istream& get_cpuinfo_stream() override {
    cpuinfo.clear();
    cpuinfo.seekg(0, std::ios::beg);
    return cpuinfo;
  }
  std::istream& get_meminfo_stream() override {
    meminfo.clear();
    meminfo.seekg(0, std::ios::beg);
    return meminfo;
  }
  std::istream& get_uptime_stream() override {
    uptime.clear();
    uptime.seekg(0, std::ios::beg);
    return uptime;
  }
  std::istream& get_stat_stream() override {
    stat.clear();
    stat.seekg(0, std::ios::beg);
    return stat;
  }
  std::istream& get_mounts_stream() override {
    mounts.clear();
    mounts.seekg(0, std::ios::beg);
    return mounts;
  }
  std::istream& get_diskstats_stream() override {
    diskstats.clear();
    diskstats.seekg(0, std::ios::beg);
    return diskstats;
  }
  uint64_t get_used_space_bytes(const std::string& mount_point) override;
  uint64_t get_disk_size_bytes(const std::string& mount_point) override;
};

LocalDataStreams get_local_file_streams();

// uint64_t LocalDataStreams::get_used_space_bytes(const std::string
// &mount_point); uint64_t LocalDataStreams::get_disk_size_bytes(const
// std::string &mount_point);
