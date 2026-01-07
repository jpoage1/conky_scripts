#include "data_ssh.hpp"

#include "lua_generator.hpp"
#include "provider.hpp"
#include "ssh.hpp"
#include "stream_provider.hpp"

namespace telemetry {

ProcDataStreams::ProcDataStreams() {
  if (setup_ssh_session() != 0) {
    std::cerr << "Failed to set up default SSH session. Exiting." << std::endl;
    return;
  }
}

/**
 * @brief Overloaded function to get metrics from a specific SSH server.
 */
ProcDataStreams::ProcDataStreams(const std::string &host,
                                 const std::string &user) {
  if (setup_ssh_session(host, user) != 0) {
    std::cerr << "Failed to set up SSH session to " << user << "@" << host
              << ". Exiting." << std::endl;
    return;
  }
}

std::stringstream &
ProcDataStreams::create_stream_from_command(std::stringstream &stream,
                                            const char *cmd) {
  std::string data = execute_ssh_command(cmd);
  stream.str(data);
  rewind(stream, "uptime");
  return stream;
}
// Helper function to trim leading and trailing whitespace.
std::string trim(const std::string &str) {
  const auto str_begin = str.find_first_not_of(" \t\n\r\f\v");
  if (std::string::npos == str_begin) {
    return "";
  }
  const auto str_end = str.find_last_not_of(" \t\n\r\f\v");
  const auto str_range = str_end - str_begin + 1;
  return str.substr(str_begin, str_range);
}

void ProcDataStreams::cleanup() { cleanup_ssh_session(); }

// uint64_t get_df_data_bytes(const std::string& mount_point, bool get_used) {
//   // Execute df command and get the output as a single string.
//   std::string df_output = execute_ssh_command("df -B1 " + mount_point);

//   // Check if the command was successful. df returns an error if the mount
//   point
//   // is not found.
//   if (df_output.empty()) {
//     std::cerr << "Error: Could not get df output for mount point "
//               << mount_point << std::endl;
//     return 0;
//   }

//   std::stringstream df_stream(df_output);
//   std::string line;
//   std::getline(df_stream, line);  // Read and discard the header line.

//   std::getline(df_stream, line);  // Read the data line.

//   std::stringstream data_stream(line);
//   std::string filesystem, blocks, used, available, capacity, mounted_on;

//   // Parse the data line.
//   if (data_stream >> filesystem >> blocks >> used >> available >> capacity >>
//       mounted_on) {
//     // df might return multiple lines if the mount point is a symbolic link.
//     // We'll trust the first data line.
//     try {
//       if (get_used) {
//         return std::stoull(used);
//       } else {
//         return std::stoull(blocks);
//       }
//     } catch (const std::exception& e) {
//       std::cerr << "Error parsing numbers from df output: " << e.what()
//                 << std::endl;
//       return 0;
//     }
//   }

//   std::cerr << "Error: Could not parse df output for mount point "
//             << mount_point << std::endl;
//   return 0;
// }

// uint64_t ProcDataStreams::get_used_space_bytes(const std::string&
// mount_point) {
//   return get_df_data_bytes(mount_point, true);
// }

// uint64_t ProcDataStreams::get_disk_size_bytes(const std::string& mount_point)
// {
//   return get_df_data_bytes(mount_point, false);
// }

std::string LuaSSH::serialize(unsigned indentation_level) const {
  LuaConfigGenerator gen("ssh", indentation_level);

  gen.lua_bool("enabled", enabled);
  gen.lua_string("host", host);
  gen.lua_string("user", user);
  gen.lua_string("key_path", key_path);
  gen.lua_int("timeout_sec", timeout_sec);
  gen.lua_bool("keepalive", keepalive);

  return gen.str();
} // End SSH::serialize

void LuaSSH::deserialize(sol::table ssh) {
  if (!ssh.valid())
    return;
  enabled = ssh.get_or("enabled", false);
  host = ssh.get_or("host", std::string(""));
  user = ssh.get_or("user", std::string(""));
  key_path = ssh.get_or("key_path", std::string("~/.ssh/id_rsa"));
  timeout_sec = ssh.get_or("timeout_sec", 5);
  keepalive = ssh.get_or("keepalive", true);
}

}; // namespace telemetry
