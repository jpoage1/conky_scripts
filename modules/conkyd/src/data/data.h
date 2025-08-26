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

struct LocalDataStreams {
  std::ifstream cpuinfo;
  std::ifstream meminfo;
  std::ifstream uptime;
  std::ifstream stat;
};

SystemMetrics read_data(std::ifstream &, std::ifstream &, std::ifstream &,
                        std::ifstream &);
void print_metrics(SystemMetrics);
