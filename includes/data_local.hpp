// data_local.h
#pragma once
#include "pcn.hpp"
#include "stream_provider.hpp"

struct PopenDeleter {
  void operator()(FILE* fp) const {
    if (fp) {
      pclose(fp);
    }
  }
};
struct DiskUsage;
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

  /* DataStreamProvider functions */
  std::istream& get_cpuinfo_stream() override;
  std::istream& get_meminfo_stream() override;
  std::istream& get_uptime_stream() override;
  std::istream& get_stat_stream() override;  // cpu stats
  std::istream& get_mounts_stream() override;
  std::istream& get_diskstats_stream() override;
  std::istream& get_loadavg_stream() override;
  std::istream& get_net_dev_stream() override;
  ProcessSnapshotMap get_process_snapshots() override;

  //   std::istream& get_top_mem_processes_stream() override;
  //   std::istream& get_top_cpu_processes_stream() override;
  //   uint64_t get_used_space_bytes(const std::string& mount_point) override;
  //   uint64_t get_disk_size_bytes(const std::string& mount_point) override;
  void finally() override;

  DiskUsage get_disk_usage(const std::string&) override;

  double get_cpu_temperature() override;

  /* LocalDataStreams functions */
  std::ifstream& create_stream_from_file(std::ifstream& stream,
                                         const std::string& path);
  std::string exec_local_cmd(const char* cmd);
  std::optional<std::string> read_sysfs_file(const std::filesystem::path& path);
  std::stringstream& create_stream_from_command(std::stringstream& stream,
                                                const char* cmd);
  std::stringstream& create_stream_from_command(std::stringstream& stream,
                                                const char* cmd,
                                                std::string stream_name);
};

using LocalDataStreamsPtr = std::unique_ptr<LocalDataStreams>;
