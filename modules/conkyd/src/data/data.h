#pragma once
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <vector>

struct SystemMetrics {
  struct CoreStats {
    size_t core_id;
    long unsigned idle_time;
    long unsigned total_time;
  };

  std::vector<CoreStats> cores;
  double cpu_frequency_ghz;
  long mem_used_kb;
  long mem_total_kb;
  int mem_percent;
  long swap_used_kb;
  long swap_total_kb;
  int swap_percent;
  std::string uptime;
};

class DataStreamProvider {
 public:
  virtual ~DataStreamProvider() = default;
  virtual std::istream &get_cpuinfo_stream() = 0;
  virtual std::istream &get_meminfo_stream() = 0;
  virtual std::istream &get_uptime_stream() = 0;
  virtual std::istream &get_stat_stream() = 0;
  virtual std::istream &get_mounts_stream() = 0;
  virtual std::istream &get_diskstats_stream() = 0;
};
SystemMetrics read_data(DataStreamProvider &);

void print_metrics(SystemMetrics);
