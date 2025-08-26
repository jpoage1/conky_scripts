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
};

// struct ProcDataStreams : public DataStreamProvider {
//   std::stringstream cpuinfo;
//   std::stringstream meminfo;
//   std::stringstream uptime;
//   std::stringstream stat;

//   std::istream &get_cpuinfo_stream() override { return cpuinfo; }
//   std::istream &get_meminfo_stream() override { return meminfo; }
//   std::istream &get_uptime_stream() override { return uptime; }
//   std::istream &get_stat_stream() override { return stat; }
// };
// struct LocalDataStreams : public DataStreamProvider {
//   std::ifstream cpuinfo;
//   std::ifstream meminfo;
//   std::ifstream uptime;
//   std::ifstream stat;

//   std::istream &get_cpuinfo_stream() override { return cpuinfo; }
//   std::istream &get_meminfo_stream() override { return meminfo; }
//   std::istream &get_uptime_stream() override { return uptime; }
//   std::istream &get_stat_stream() override { return stat; }
// };

struct LocalDataStreams {
  std::ifstream cpuinfo;
  std::ifstream meminfo;
  std::ifstream uptime;
  std::ifstream stat;
};

void print_data(SystemMetrics);

SystemMetrics read_data(LocalDataStreams &);
// ProcDataStreams get_ssh_streams();
LocalDataStreams get_local_file_streams();

void print_metrics(SystemMetrics);
