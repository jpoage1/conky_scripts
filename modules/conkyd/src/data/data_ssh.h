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

ProcDataStreams get_ssh_streams();

std::string trim(const std::string& str);
uint64_t get_df_data_bytes(const std::string& mount_point, bool get_used);

// uint64_t ProcDataStreams::get_used_space_bytes(const std::string
// &mount_point); uint64_t ProcDataStreams::get_disk_size_bytes(const
// std::string &mount_point);
