#include <sys/statvfs.h>

#include <string>

#include "data.h"

struct LocalDataStreams : public DataStreamProvider {
  std::ifstream cpuinfo;
  std::ifstream meminfo;
  std::ifstream uptime;
  std::ifstream stat;
  std::ifstream mounts;
  std::ifstream diskstats;
  std::ifstream loadavg;
  std::ifstream net_dev;
  std::stringstream top_mem_procs;
  std::stringstream top_cpu_procs;

  void reset_stream(std::ifstream& stream, const std::string& path) {
      if (stream.is_open()) {
          stream.close();
      }
      stream.open(path);
      if (!stream.is_open()) {
          std::cerr << "[Error] Failed to open " << path << std::endl;
      }
  }

    std::istream& get_cpuinfo_stream() override {
    reset_stream(cpuinfo, "/proc/cpuinfo");
    return cpuinfo;
  }
  std::istream& get_meminfo_stream() override {
    reset_stream(meminfo, "/proc/meminfo");
    return meminfo;
  }
  std::istream& get_uptime_stream() override {
    reset_stream(uptime, "/proc/uptime");
    return uptime;
  }
  std::istream& get_stat_stream() override {
    reset_stream(stat, "/proc/stat");
    return stat;
  }
  std::istream& get_mounts_stream() override {
    reset_stream(mounts, "/proc/mounts");
    return mounts;
  }
  std::istream& get_diskstats_stream() override {
    reset_stream(diskstats, "/proc/diskstats");
    return diskstats;
  }
  std::istream& get_loadavg_stream() override {
    reset_stream(loadavg, "/proc/loadavg");
    return loadavg;
  }
  std::istream& get_net_dev_stream() override {
    reset_stream(net_dev, "/proc/net/dev");
    return net_dev;
  }
  std::istream& get_top_mem_processes_stream() override;
  std::istream& get_top_cpu_processes_stream() override;
  uint64_t get_used_space_bytes(const std::string& mount_point) override;
  uint64_t get_disk_size_bytes(const std::string& mount_point) override;

  double get_cpu_temperature() override;
};

// uint64_t LocalDataStreams::get_used_space_bytes(const std::string
// &mount_point); uint64_t LocalDataStreams::get_disk_size_bytes(const
// std::string &mount_point);
