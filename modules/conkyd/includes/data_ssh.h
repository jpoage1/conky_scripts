// data_ssh.h
#pragma once
#include "data.h"
#include "pcn.hpp"
struct DiskUsage;
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

  /*  DataStreamProvider functions */

  std::istream& get_cpuinfo_stream() override;
  std::istream& get_meminfo_stream() override;
  std::istream& get_uptime_stream() override;
  std::istream& get_stat_stream() override;
  std::istream& get_mounts_stream() override;
  std::istream& get_diskstats_stream() override;
  std::istream& get_loadavg_stream() override;
  std::istream& get_net_dev_stream() override;
  //   std::istream& get_top_mem_processes_stream() override;
  //   std::istream& get_top_cpu_processes_stream() override;
  //   uint64_t get_used_space_bytes(const std::string& mount_point) override;
  //   uint64_t get_disk_size_bytes(const std::string& mount_point) override;
  std::istream& get_top_mem_processes_avg_stream() override;
  std::istream& get_top_cpu_processes_avg_stream() override;
  std::istream& get_top_mem_processes_real_stream() override;
  std::istream& get_top_cpu_processes_real_stream() override;
  DiskUsage get_disk_usage(const std::string&) override;

  /* ProcDataStreams functions */
  ProcDataStreams() {}
  double get_cpu_temperature() override { return -1.0; }
  std::stringstream& create_stream_from_command(std::stringstream& stream,
                                                const char* cmd);
  std::stringstream& create_stream_from_command(std::stringstream& stream,
                                                const char* cmd,
                                                std::string stream_name);
};
std::string trim(const std::string& str);
uint64_t get_df_data_bytes(const std::string& mount_point, bool get_used);
