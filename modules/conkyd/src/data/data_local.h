#include "data.h"

struct LocalDataStreams : public DataStreamProvider {
  std::ifstream cpuinfo;
  std::ifstream meminfo;
  std::ifstream uptime;
  std::ifstream stat;
  std::ifstream mounts;
  std::ifstream diskstats;

  std::istream &get_cpuinfo_stream() override { return cpuinfo; }
  std::istream &get_meminfo_stream() override { return meminfo; }
  std::istream &get_uptime_stream() override { return uptime; }
  std::istream &get_stat_stream() override { return stat; }
  std::istream &get_mounts_stream() override { return mounts; }
  std::istream &get_diskstats_stream() override { return diskstats; }
};

LocalDataStreams get_local_file_streams();
