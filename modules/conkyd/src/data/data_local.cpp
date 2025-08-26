#include "data_local.h"

LocalDataStreams get_local_file_streams() {
  LocalDataStreams streams;
  streams.cpuinfo.open("/proc/cpuinfo");
  streams.meminfo.open("/proc/meminfo");
  streams.uptime.open("/proc/uptime");
  streams.stat.open("/proc/stat");
  return streams;
}
