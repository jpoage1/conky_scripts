#include "data_ssh.h"

ProcDataStreams get_ssh_streams() {
  // Get the remote data
  std::string cpu_data = execute_ssh_command("cat /proc/cpuinfo");
  std::string meminfo_data = execute_ssh_command("cat /proc/meminfo");
  std::string uptime_data = execute_ssh_command("cat /proc/uptime");
  std::string stat_data = execute_ssh_command("cat /proc/stat");

  // Create string streams from the retrieved data
  std::stringstream cpu_file_stream(cpu_data);
  std::stringstream meminfo_file_stream(meminfo_data);
  std::stringstream uptime_file_stream(uptime_data);
  std::stringstream stat_file_stream(stat_data);

  ProcDataStreams streams;
  streams.cpuinfo << cpu_data;
  streams.meminfo << meminfo_data;
  streams.uptime << uptime_data;
  streams.stat << stat_data;

  return streams;
}
