#include "data.h"

struct LocalDataStreams : public DataStreamProvider {
  std::ifstream cpuinfo;
  std::ifstream meminfo;
  std::ifstream uptime;
  std::ifstream stat;

  std::istream &get_cpuinfo_stream() override { return cpuinfo; }
  std::istream &get_meminfo_stream() override { return meminfo; }
  std::istream &get_uptime_stream() override { return uptime; }
  std::istream &get_stat_stream() override { return stat; }
};

LocalDataStreams get_local_file_streams();
