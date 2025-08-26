#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <vector>

#include "data.h"
#include "ssh.h"

struct ProcDataStreams : public DataStreamProvider {
  std::stringstream cpuinfo;
  std::stringstream meminfo;
  std::stringstream uptime;
  std::stringstream stat;

  std::istream &get_cpuinfo_stream() override { return cpuinfo; }
  std::istream &get_meminfo_stream() override { return meminfo; }
  std::istream &get_uptime_stream() override { return uptime; }
  std::istream &get_stat_stream() override { return stat; }
};

SystemMetrics read_data(ProcDataStreams &);
SystemMetrics read_data(DataStreamProvider &);

ProcDataStreams get_ssh_streams();
