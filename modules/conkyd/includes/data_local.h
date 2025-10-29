#include <sys/statvfs.h>

#include <string>

#include "data.h"

void rewind(std::ifstream& stream, const std::string&);

struct LocalDataStreams : public DataStreamProvider {
  std::ifstream cpuinfo;
  std::ifstream meminfo;
  std::ifstream uptime;
  std::ifstream stat;
  std::ifstream mounts;
  std::ifstream diskstats;
  std::ifstream loadavg;
  std::ifstream net_dev;
  std::stringstream top_mem_procs_stream;

  std::istream& get_cpuinfo_stream() override { return cpuinfo; }
  std::istream& get_meminfo_stream() override {
    rewind(meminfo, "meminfo");
    return meminfo;
  }
  std::istream& get_uptime_stream() override {
    rewind(uptime, "uptime");
    return uptime;
  }
  std::istream& get_stat_stream() override {
    rewind(stat, "stat");
    return stat;
  }
  std::istream& get_mounts_stream() override {
    rewind(mounts, "mounts");
    return mounts;
  }
  std::istream& get_diskstats_stream() override {
    rewind(diskstats, "diskstats");
    return diskstats;
  }
  std::istream& get_loadavg_stream() override {
    rewind(loadavg, "loadavg");
    return loadavg;
  }
  //   std::istream& get_net_dev_stream() override {
  //     rewind(net_dev);
  //     return net_dev;
  //   }
  std::istream& get_net_dev_stream() override {
    // Close if already open (might be redundant if closed in constructor,
    // safe)
    if (net_dev.is_open()) {
      net_dev.close();
    }
    // Re-open the file fresh each time this is called
    net_dev.open("/proc/net/dev");

    if (!net_dev.is_open()) {
      std::cerr
          << "[Error] Failed to re-open /proc/net/dev in get_net_dev_stream."
          << std::endl;
      // Stream will be in fail state
    }
    // No need for clear() or seekg() on a fresh open
    return net_dev;
  }
  //   std::istream& get_top_mem_processes_stream() override;
  uint64_t get_used_space_bytes(const std::string& mount_point) override;
  uint64_t get_disk_size_bytes(const std::string& mount_point) override;

  double get_cpu_temperature() override;
};

LocalDataStreams get_local_file_streams();

// uint64_t LocalDataStreams::get_used_space_bytes(const std::string
// &mount_point); uint64_t LocalDataStreams::get_disk_size_bytes(const
// std::string &mount_point);
