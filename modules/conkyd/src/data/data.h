#pragma once
#include <cstdint>
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <vector>

#include "corestat.h"
#include "networkstats.hpp"
#include "processinfo.hpp"

void rewind(std::stringstream& stream, const std::string&);

struct SystemMetrics {
  std::vector<CoreStats> cores;
  double cpu_frequency_ghz;
  double cpu_temp_c;
  long mem_used_kb;
  long mem_total_kb;
  int mem_percent;
  long swap_used_kb;
  long swap_total_kb;
  int swap_percent;
  std::string uptime;

  double load_avg_1m = 0.0;
  double load_avg_5m = 0.0;
  double load_avg_15m = 0.0;
  long processes_total = 0;
  long processes_running = 0;

  std::string sys_name;
  std::string node_name;
  std::string kernel_release;
  std::string machine_type;
  std::vector<NetworkInterfaceStats> network_interfaces;
};

class DataStreamProvider {
 public:
  virtual ~DataStreamProvider() = default;
  virtual std::istream& get_cpuinfo_stream() = 0;
  virtual std::istream& get_meminfo_stream() = 0;
  virtual std::istream& get_uptime_stream() = 0;
  virtual std::istream& get_stat_stream() = 0;
  virtual std::istream& get_mounts_stream() = 0;
  virtual std::istream& get_diskstats_stream() = 0;
  virtual std::istream& get_loadavg_stream() = 0;
  virtual std::istream& get_net_dev_stream() = 0;
  virtual uint64_t get_used_space_bytes(const std::string& mount_point) = 0;
  virtual uint64_t get_disk_size_bytes(const std::string& mount_point) = 0;
  virtual double get_cpu_temperature() = 0;
};
SystemMetrics read_data(DataStreamProvider&);

void print_metrics(const SystemMetrics&);
void get_load_and_process_stats(DataStreamProvider& provider,
                                SystemMetrics& metrics);
