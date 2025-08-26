#include "data_ssh.h"

ProcDataStreams get_ssh_streams() {
  // Get the remote data
  std::string cpu_data = execute_ssh_command("cat /proc/cpuinfo");
  std::string meminfo_data = execute_ssh_command("cat /proc/meminfo");
  std::string uptime_data = execute_ssh_command("cat /proc/uptime");
  std::string stat_data = execute_ssh_command("cat /proc/stat");
  std::string mounts_data = execute_ssh_command("cat /proc/mounts");
  std::string diskstats_data = execute_ssh_command("cat /proc/diskstats");

  // Create string streams from the retrieved data
  std::stringstream cpu_file_stream(cpu_data);
  std::stringstream meminfo_file_stream(meminfo_data);
  std::stringstream uptime_file_stream(uptime_data);
  std::stringstream stat_file_stream(stat_data);
  std::stringstream mounts_file_stream(mounts_data);
  std::stringstream diskstats_file_stream(diskstats_data);

  ProcDataStreams streams;
  streams.cpuinfo << cpu_data;
  streams.meminfo << meminfo_data;
  streams.uptime << uptime_data;
  streams.stat << stat_data;
  streams.mounts << mounts_data;
  streams.diskstats << diskstats_data;

  return streams;
}
