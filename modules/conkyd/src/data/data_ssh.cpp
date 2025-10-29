#include "data_ssh.h"

ProcDataStreams get_ssh_streams() {
  // Get the remote data
  std::string cpu_data = execute_ssh_command("cat /proc/cpuinfo");
  std::string meminfo_data = execute_ssh_command("cat /proc/meminfo");
  std::string uptime_data = execute_ssh_command("cat /proc/uptime");
  std::string stat_data = execute_ssh_command("cat /proc/stat");
  std::string mounts_data = execute_ssh_command("cat /proc/mounts");
  std::string diskstats_data = execute_ssh_command("cat /proc/diskstats");
  std::string loadavg_data = execute_ssh_command("cat /proc/loadavg");
  std::string net_dev_data = execute_ssh_command("cat /proc/net/dev");
  std::string top_mem_data = execute_ssh_command(
      "ps -eo pid,rss,comm --no-headers --sort=-rss | head -n 10");

  // Create string streams from the retrieved data
  std::stringstream cpu_file_stream(cpu_data);
  std::stringstream meminfo_file_stream(meminfo_data);
  std::stringstream uptime_file_stream(uptime_data);
  std::stringstream stat_file_stream(stat_data);
  std::stringstream mounts_file_stream(mounts_data);
  std::stringstream diskstats_file_stream(diskstats_data);
  std::stringstream loadavg_file_stream(loadavg_data);
  std::stringstream net_dev_file_stream(net_dev_data);

  ProcDataStreams streams;
  streams.cpuinfo << cpu_data;
  streams.meminfo << meminfo_data;
  streams.uptime << uptime_data;
  streams.stat << stat_data;
  streams.mounts << mounts_data;
  streams.diskstats << diskstats_data;
  streams.loadavg << loadavg_data;
  streams.net_dev << net_dev_data;
  //   streams.top_mem_procs_stream << top_mem_data;

  return streams;
}

// Helper function to trim leading and trailing whitespace.
std::string trim(const std::string& str) {
  const auto str_begin = str.find_first_not_of(" \t\n\r\f\v");
  if (std::string::npos == str_begin) {
    return "";
  }
  const auto str_end = str.find_last_not_of(" \t\n\r\f\v");
  const auto str_range = str_end - str_begin + 1;
  return str.substr(str_begin, str_range);
}

uint64_t get_df_data_bytes(const std::string& mount_point, bool get_used) {
  // Execute df command and get the output as a single string.
  std::string df_output = execute_ssh_command("df -B1 " + mount_point);

  // Check if the command was successful. df returns an error if the mount point
  // is not found.
  if (df_output.empty()) {
    std::cerr << "Error: Could not get df output for mount point "
              << mount_point << std::endl;
    return 0;
  }

  std::stringstream df_stream(df_output);
  std::string line;
  std::getline(df_stream, line);  // Read and discard the header line.

  std::getline(df_stream, line);  // Read the data line.

  std::stringstream data_stream(line);
  std::string filesystem, blocks, used, available, capacity, mounted_on;

  // Parse the data line.
  if (data_stream >> filesystem >> blocks >> used >> available >> capacity >>
      mounted_on) {
    // df might return multiple lines if the mount point is a symbolic link.
    // We'll trust the first data line.
    try {
      if (get_used) {
        return std::stoull(used);
      } else {
        return std::stoull(blocks);
      }
    } catch (const std::exception& e) {
      std::cerr << "Error parsing numbers from df output: " << e.what()
                << std::endl;
      return 0;
    }
  }

  std::cerr << "Error: Could not parse df output for mount point "
            << mount_point << std::endl;
  return 0;
}

uint64_t ProcDataStreams::get_used_space_bytes(const std::string& mount_point) {
  return get_df_data_bytes(mount_point, true);
}

uint64_t ProcDataStreams::get_disk_size_bytes(const std::string& mount_point) {
  return get_df_data_bytes(mount_point, false);
}
