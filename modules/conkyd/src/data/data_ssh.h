#include <istream>
#include <sstream>

#include "data.h"
#include "ssh.h"

struct ProcDataStreams : public DataStreamProvider {
  std::stringstream cpuinfo;
  std::stringstream meminfo;
  std::stringstream uptime;
  std::stringstream stat;
  std::stringstream mounts;
  std::stringstream diskstats;

  std::istream &get_cpuinfo_stream() override { return cpuinfo; }
  std::istream &get_meminfo_stream() override { return meminfo; }
  std::istream &get_uptime_stream() override { return uptime; }
  std::istream &get_stat_stream() override { return stat; }
  std::istream &get_mounts_stream() override { return mounts; }
  std::istream &get_diskstats_stream() override { return diskstats; }
};

ProcDataStreams get_ssh_streams();
