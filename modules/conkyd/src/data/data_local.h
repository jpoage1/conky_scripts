#include "data.h"

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
// // Works
// SystemMetrics read_data(std::ifstream &cpu_file_stream,
//                         std::ifstream &meminfo_file_stream,
//                         std::ifstream &uptime_file_stream,
//                         std::ifstream &stat_file_stream);
// Doesn't work
SystemMetrics read_data(LocalDataStreams &);

LocalDataStreams get_local_file_streams();
