#include <sys/statvfs.h>

#include "data.h"

void rewind(std::ifstream& stream);

struct LocalDataStreams : public DataStreamProvider {
  std::ifstream cpuinfo;
  std::ifstream meminfo;
  std::ifstream uptime;
  std::ifstream stat;
  std::ifstream mounts;
  std::ifstream diskstats;
  std::ifstream loadavg;
  std::ifstream net_dev;

  std::istream& get_cpuinfo_stream() override {
    rewind(cpuinfo);
    return cpuinfo;
  }
  std::istream& get_meminfo_stream() override {
    rewind(meminfo);
    return meminfo;
  }
  std::istream& get_uptime_stream() override {
    rewind(uptime);
    return uptime;
  }
  std::istream& get_stat_stream() override {
    rewind(stat);
    return stat;
  }
  std::istream& get_mounts_stream() override {
    rewind(mounts);
    return mounts;
  }
  std::istream& get_diskstats_stream() override {
    rewind(diskstats);
    return diskstats;
  }
  std::istream& get_loadavg_stream() override {
    rewind(loadavg);
    return loadavg;
  }
  std::istream& get_net_dev_stream() override {
    // Close if already open (might be redundant if closed in constructor, but
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
  uint64_t get_used_space_bytes(const std::string& mount_point) override;
  uint64_t get_disk_size_bytes(const std::string& mount_point) override;

  double get_cpu_temperature() override;
};

LocalDataStreams get_local_file_streams();

// uint64_t LocalDataStreams::get_used_space_bytes(const std::string
// &mount_point); uint64_t LocalDataStreams::get_disk_size_bytes(const
// std::string &mount_point);
