// data_ssh.h
#pragma once
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
  std::stringstream top_mem_procs;
  std::stringstream top_cpu_procs;

  ProcDataStreams() {}

  std::istream& get_cpuinfo_stream() override;
  std::istream& get_meminfo_stream() override;
  std::istream& get_uptime_stream() override;
  std::istream& get_stat_stream() override;
  std::istream& get_mounts_stream() override;
  std::istream& get_diskstats_stream() override;
  std::istream& get_loadavg_stream() override;
  std::istream& get_net_dev_stream() override;
  std::istream& get_top_mem_processes_stream() override;
  std::istream& get_top_cpu_processes_stream() override;
  uint64_t get_used_space_bytes(const std::string& mount_point) override;
  uint64_t get_disk_size_bytes(const std::string& mount_point) override;

  double get_cpu_temperature() override { return -1.0; }
};

std::string trim(const std::string& str);
uint64_t get_df_data_bytes(const std::string& mount_point, bool get_used);
