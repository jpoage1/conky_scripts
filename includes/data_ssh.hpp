// data_ssh.h
#pragma once
#include "data.hpp"
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
  ProcessSnapshotMap get_process_snapshots() override;
  //   std::istream& get_top_mem_processes_stream() override;
  //   std::istream& get_top_cpu_processes_stream() override;
  //   uint64_t get_used_space_bytes(const std::string& mount_point) override;
  //   uint64_t get_disk_size_bytes(const std::string& mount_point) override;
  DiskUsage get_disk_usage(const std::string&) override;
  void finally() override;

  /* ProcDataStreams functions */
  ProcDataStreams(const std::string& host, const std::string& user);
  ProcDataStreams();
  double get_cpu_temperature() override { return -1.0; }
  std::stringstream& create_stream_from_command(std::stringstream& stream,
                                                const char* cmd);
  std::stringstream& create_stream_from_command(std::stringstream& stream,
                                                const char* cmd,
                                                std::string stream_name);

  int setup_ssh_session(const std::string host, const std::string user);

  int ssh_connection(const std::string host, const std::string user);

  std::string execute_ssh_command(const std::string&);
  void cleanup_ssh_session();
  int ssh_connection();
  int setup_ssh_session();
};
std::string trim(const std::string& str);
uint64_t get_df_data_bytes(const std::string& mount_point, bool get_used);
