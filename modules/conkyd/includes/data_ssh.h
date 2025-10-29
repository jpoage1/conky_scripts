#include <istream>
#include <sstream>
#include <string>

#include "data.h"
#include "ssh.h"

struct ProcDataStreams : public DataStreamProvider {
  std::stringstream cpuinfo;
  std::stringstream meminfo;
  std::stringstream uptime;
  std::stringstream stat;
  std::stringstream mounts;
  std::stringstream diskstats;
  std::stringstream loadavg;
  std::stringstream net_dev;

  std::istream& get_cpuinfo_stream() override {
    rewind(cpuinfo, "cpuinfo");
    return cpuinfo;
  }
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
  std::istream& get_net_dev_stream() override {
    rewind(net_dev, "net_dev");
    return net_dev;
  }
  uint64_t get_used_space_bytes(const std::string& mount_point) override;
  uint64_t get_disk_size_bytes(const std::string& mount_point) override;

  double get_cpu_temperature() override {
    return -1.0;  // TODO: Implement SSH temp logic
  }
};

ProcDataStreams get_ssh_streams();

std::string trim(const std::string& str);
uint64_t get_df_data_bytes(const std::string& mount_point, bool get_used);

// uint64_t ProcDataStreams::get_used_space_bytes(const std::string
// &mount_point); uint64_t ProcDataStreams::get_disk_size_bytes(const
// std::string &mount_point);
