// data_local.h
#pragma once
#include <sys/statvfs.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

#include "data.h"

struct PopenDeleter {
  void operator()(FILE* fp) const {
    if (fp) {
      pclose(fp);
    }
  }
};

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

  void reset_stream(std::ifstream& stream, const std::string& path);

  std::string exec_local_cmd(const char* cmd);

  std::optional<std::string> read_sysfs_file(const std::filesystem::path& path);

  std::istream& get_cpuinfo_stream() override;
  std::istream& get_meminfo_stream() override;
  std::istream& get_uptime_stream() override;
  std::istream& get_stat_stream() override;  // cpu stats
  std::istream& get_mounts_stream() override;
  std::istream& get_diskstats_stream() override;
  std::istream& get_loadavg_stream() override;
  std::istream& get_net_dev_stream() override;

  std::istream& get_top_mem_processes_stream() override;
  std::istream& get_top_cpu_processes_stream() override;
  uint64_t get_used_space_bytes(const std::string& mount_point) override;
  uint64_t get_disk_size_bytes(const std::string& mount_point) override;

  double get_cpu_temperature() override;
};
