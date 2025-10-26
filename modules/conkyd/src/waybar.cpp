#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "data.h"
#include "data_local.h"
#include "data_ssh.h"
#include "device_info.h"
#include "diskstat.h"
#include "metrics.hpp"
#include "nlohmann/json.hpp"
#include "size_format.h"
#include "ssh.h"
#include "types.h"

// Use the nlohmann namespace
using json = nlohmann::json;

struct FormattedSize {
  std::string text;
  std::string color;
  // ... other members ...

  // Your existing .formatted() method
  std::string formatted(TargetFormat target) const {
    if (target == TargetFormat::WAYBAR) {
      return "<span foreground='#" + color + "'>" + text + "</span>";
    }
    // ... other formats ...
    return text;
  }
};
// int get_metrics(const std::string& config_file, const bool use_ssh,
//                 CombinedMetrics& metrics);
// int get_local_metrics(const std::string& config_file, CombinedMetrics&
// metrics); int get_metrics_from_provider(DataStreamProvider* provider,
//                               const std::string& config_file,
//                               CombinedMetrics& metrics);

// int get_server_metrics(const std::string& config_file,
//                        CombinedMetrics& metrics);

// int get_server_metrics(const std::string& config_file, CombinedMetrics&
// metrics,
//                        const std::string& host, const std::string& user);

// /**
//  * @brief Gets metrics from the local filesystem.
//  */
// int get_local_metrics(const std::string& config_file,
//                       CombinedMetrics& metrics) {
//   LocalDataStreams local_streams = LocalDataStreams();
//   return get_metrics_from_provider(&local_streams, config_file, metrics);
// }

// /**
//  * @brief Gets metrics from the default SSH server.
//  */
// int get_server_metrics(const std::string& config_file,
//                        CombinedMetrics& metrics) {
//   if (setup_ssh_session() != 0) {  // Calls the default version
//     std::cerr << "Failed to set up default SSH session. Exiting." <<
//     std::endl; return 2;
//   }

//   ProcDataStreams ssh_streams = ProcDataStreams();
//   int result = get_metrics_from_provider(&ssh_streams, config_file, metrics);
//   cleanup_ssh_session();
//   return result;
// }

// /**
//  * @brief Overloaded function to get metrics from a specific SSH server.
//  */
// int get_server_metrics(const std::string& config_file, CombinedMetrics&
// metrics,
//                        const std::string& host, const std::string& user) {
//   if (setup_ssh_session(host, user) != 0) {  // Calls the parameterized
//   version
//     std::cerr << "Failed to set up SSH session to " << user << "@" << host
//               << ". Exiting." << std::endl;
//     return 2;
//   }

//   ProcDataStreams ssh_streams = ProcDataStreams();
//   int result = get_metrics_from_provider(&ssh_streams, config_file, metrics);
//   cleanup_ssh_session();
//   return result;
// }

// /**
//  * @brief The new core logic function.
//  * * This private helper function contains the logic from the old get_metrics
//  * function. It is generic and operates on any DataStreamProvider.
//  */
// int get_metrics_from_provider(DataStreamProvider* provider,
//                               const std::string& config_file,
//                               CombinedMetrics& metrics) {
//   if (!provider) {
//     std::cerr << "Error: No data provider could be initialized." <<
//     std::endl; return 1;
//   }

//   std::vector<std::string> device_paths;
//   if (read_device_paths(config_file, device_paths) == 1) {
//     std::cout << "${color red}Unable to load file: " + config_file +
//     "${color}"
//               << std::endl;
//     return 1;
//   }

//   metrics.system = read_data(*provider);
//   metrics.disks = collect_device_info(*provider, device_paths);
//   return 0;
// }

// int get_metrics(const std::string& config_file, const bool use_ssh,
//                 CombinedMetrics& metrics) {
//   LocalDataStreams local_streams;
//   ProcDataStreams ssh_streams;
//   DataStreamProvider* provider = nullptr;

//   if (use_ssh) {
//     if (setup_ssh_session() != 0) {
//       std::cerr << "Failed to set up SSH session. Exiting." << std::endl;
//       return 2;
//     }
//     ssh_streams = get_ssh_streams();
//     provider = &ssh_streams;
//   } else {
//     local_streams = get_local_file_streams();
//     provider = &local_streams;
//   }
//   std::vector<std::string> device_paths;
//   if (read_device_paths(config_file, device_paths) == 1) {
//     std::cout << "${color red}Unable to load file: " + config_file +
//     "${color}"
//               << std::endl;
//     return 1;
//   }

//   if (provider) {
//     metrics.system = read_data(*provider);
//     metrics.disks = collect_device_info(*provider, device_paths);
//   } else {
//     std::cerr << "Error: No data provider could be initialized." <<
//     std::endl; return 1;
//   }
//   if (use_ssh) {
//     cleanup_ssh_session();
//   }
//   return 0;
// }
//================================================================================
// WAYBAR OUTPUT GENERATOR
//================================================================================
// void generate_waybar_output(const CombinedMetrics& all_metrics) {
//   const auto& metrics = all_metrics.system;
//   const auto& devices = all_metrics.disks;
//   const TargetFormat target = TargetFormat::WAYBAR;  // Define our target

//   json waybar_output;

//   // --- Main Text ---
//   std::stringstream text_ss;
//   //   text_ss << " " << "N/A" /* TODO: CPU % */ << "% | "
//   //           << " " << metrics.mem_percent << "%";
//   text_ss << " " << "Stats";
//   waybar_output["text"] = text_ss.str();

//   // --- Tooltip ---
//   std::stringstream tooltip_ss;
//   tooltip_ss << "<b>System Information</b>\n"
//              << "Uptime: " << metrics.uptime << "\n"
//              << "CPU Freq: " << std::fixed << std::setprecision(2)
//              << metrics.cpu_frequency_ghz << " GHz\n"
//              << "Memory: "
//              << format_size(metrics.mem_used_kb * 1024).formatted(target)
//              << " / "
//              << format_size(metrics.mem_total_kb * 1024).formatted(target)
//              << " (" << metrics.mem_percent << "%)\n\n";

//   tooltip_ss << "<b>Filesystem Usage</b>\n";
//   tooltip_ss << std::left << std::setw(15) << "Mount" << std::setw(25) <<
//   "Used"
//              << std::setw(25) << "Total" << "\n";

//   for (const auto& dev : devices) {
//     uint64_t used_percent =
//         (dev.size_bytes == 0) ? 0 : (dev.used_bytes * 100) / dev.size_bytes;
//     tooltip_ss << std::left << std::setw(15) << dev.mount_point <<
//     std::setw(25)
//                << format_size(dev.used_bytes).formatted(target) <<
//                std::setw(25)
//                << format_size(dev.size_bytes).formatted(target) << "("
//                << used_percent << "%)\n";
//   }
//   waybar_output["tooltip"] = tooltip_ss.str();

//   // --- CSS Class ---
//   if (metrics.mem_percent > 80) {
//     waybar_output["class"] = "critical";
//   } else if (metrics.mem_percent > 60) {
//     waybar_output["class"] = "warning";
//   }

//   std::cout << waybar_output.dump() << std::endl;
// }
void generate_waybar_output(const CombinedMetrics& all_metrics) {
  const auto& metrics = all_metrics.system;
  const auto& devices = all_metrics.disks;
  const TargetFormat target = TargetFormat::WAYBAR;

  json waybar_output;

  // --- Main Text ---
  std::stringstream text_ss;
  text_ss << " " << "Stats";
  waybar_output["text"] = text_ss.str();

  // --- Tooltip ---
  std::stringstream tooltip_ss;
  tooltip_ss << "<b>System Information</b>\n"
             << "Uptime: " << metrics.uptime << "\n"
             << "CPU Freq: " << std::fixed << std::setprecision(2)
             << metrics.cpu_frequency_ghz << " GHz\n"
             << "Memory: "
             << format_size(metrics.mem_used_kb * 1024).formatted(target)
             << " / "
             << format_size(metrics.mem_total_kb * 1024).formatted(target)
             << " (" << metrics.mem_percent << "%)\n\n";

  tooltip_ss << "<b>Filesystem Usage</b>\n";
  tooltip_ss << "<tt>";  // Wrap in <tt> for monospaced font

  // --- START NEW LOGIC: Find max mount point length ---
  // 1. First pass: Find the longest mount point name.
  //    Start with 5, the length of the "Mount" header.
  size_t max_mount_len = 5;
  for (const auto& dev : devices) {
    if (dev.mount_point.length() > max_mount_len) {
      max_mount_len = dev.mount_point.length();
    }
  }
  // Add 1 for a padding space
  const size_t mount_col_width = max_mount_len + 1;
  // --- END NEW LOGIC ---

  // Define other column widths
  const size_t data_col_width = 9;  // Width for "999.9 GiB"
  const std::string used_header = "Used";
  const std::string total_header = "Total";

  // 2. Create the header using the dynamic width
  tooltip_ss << std::left << std::setw(mount_col_width) << "Mount"
             << std::setw(data_col_width + 1) << used_header   // +1 for space
             << std::setw(data_col_width + 1) << total_header  // +1 for space
             << " Used%\n";

  // 3. Second pass: Build the table rows
  for (const auto& dev : devices) {
    if (dev.mount_point.empty()) continue;  // Skip if mount point wasn't found

    uint64_t used_percent =
        (dev.size_bytes == 0) ? 0 : (dev.used_bytes * 100) / dev.size_bytes;

    // Get the raw, unformatted data
    auto fs_used = format_size(dev.used_bytes);
    auto fs_total = format_size(dev.size_bytes);

    // Pad the RAW text strings
    std::stringstream used_text_ss;
    used_text_ss << std::left << std::setw(data_col_width) << fs_used.text;

    std::stringstream total_text_ss;
    total_text_ss << std::left << std::setw(data_col_width) << fs_total.text;

    // Manually wrap the PADDED text in Pango spans
    tooltip_ss << std::left
               // Use the calculated max width, no truncation
               << std::setw(mount_col_width) << dev.mount_point

               << "<span foreground='#" << fs_used.color << "'>"
               << used_text_ss.str() << "</span> "  // Manually add the space

               << "<span foreground='#" << fs_total.color << "'>"
               << total_text_ss.str() << "</span> "  // Manually add the space

               << "(" << used_percent << "%)\n";
  }

  tooltip_ss << "</tt>";  // Close monospaced tag
  waybar_output["tooltip"] = tooltip_ss.str();

  // --- CSS Class ---
  if (metrics.mem_percent > 80) {
    waybar_output["class"] = "critical";
  } else if (metrics.mem_percent > 60) {
    waybar_output["class"] = "warning";
  }

  std::cout << waybar_output.dump() << std::endl;
}
void print_usage(const char* prog_name) {
  std::cerr
      << "Usage: " << prog_name << " [options...]\n\n"
      << "Generates metrics based on one or more commands.\n"
      << "If the first argument is a file path, it defaults to --local.\n\n"
      << "Commands:\n"
      << "  <config_file>       (As first argument) Generate local metrics.\n"
      << "  --local <config_file>\n"
      << "                      Generate local metrics.\n"
      << "  --ssh <config_file>\n"
      << "                      Generate metrics from default SSH host.\n"
      << "  --ssh <config_file> <host> <user>\n"
      << "                      Generate metrics from specific SSH host.\n\n"
      << "Example:\n"
      << "  " << prog_name
      << " /path/local.conf --ssh /path/ssh.conf my-server conky\n";
}
int check_config_file(const std::string& config_file) {
  if (!std::filesystem::exists(config_file)) {
    // Fulfills your requirement: "provide error, but continue processing"
    std::cerr << "Warning: Config file not found, skipping: " << config_file
              << std::endl;
    return 1;  // Return 1 to indicate an error
  }
  return 0;  // Success
}

int main(int argc, char* argv[]) {
  // Need at least program name + config_file
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  // Convert C-style argv to a C++ vector for easier parsing
  std::vector<std::string> args(argv, argv + argc);

  bool processed_at_least_one = false;

  // Start at 1 to skip the program name
  long unsigned int i = 1;
  while (i < args.size()) {
    std::string command = args[i];
    std::string config_file;

    // --- Check for Implicit Local (First argument only) ---
    if (i == 1 && command.rfind("--", 0) != 0) {
      config_file = command;  // The first arg IS the config file
      if (check_config_file(config_file) == 0) {
        waybar_local_metrics(config_file);
        processed_at_least_one = true;
      }
      i += 1;  // Consume 1 arg: <config_file>

      // --- Check for Explicit --local ---
    } else if (command == "--local") {
      if (i + 1 >= args.size()) {  // Check if config_file is missing
        std::cerr << "Error: --local requires a <config_file> argument."
                  << std::endl;
        return 1;  // Fatal parsing error
      }
      config_file = args[i + 1];
      if (check_config_file(config_file) == 0) {
        waybar_local_metrics(config_file);
        processed_at_least_one = true;
      }
      i += 2;  // Consume 2 args: --local, <config_file>

      // --- Check for --ssh ---
    } else if (command == "--ssh") {
      if (i + 1 >= args.size()) {  // Check if config_file is missing
        std::cerr << "Error: --ssh requires a <config_file> argument."
                  << std::endl;
        return 1;  // Fatal parsing error
      }
      config_file = args[i + 1];

      // Check if host and user are provided
      if (i + 3 < args.size() && args[i + 2].rfind("--", 0) != 0) {
        // --- Custom SSH: --ssh <config> <host> <user> ---
        // The next arg (args[i+2]) exists and does NOT start with "--",
        // so it must be a host.
        std::string host = args[i + 2];
        std::string user = args[i + 3];

        if (check_config_file(config_file) == 0) {
          waybar_server_metrics(config_file, host, user);
          processed_at_least_one = true;
        }
        i += 4;  // Consume 4 args: --ssh, <config_file>, <host>, <user>
      } else {
        // --- Default SSH: --ssh <config> ---
        // Either not enough args for host/user, or the next arg is a new flag
        if (check_config_file(config_file) == 0) {
          waybar_server_metrics(config_file);
          processed_at_least_one = true;
        }
        i += 2;  // Consume 2 args: --ssh, <config_file>
      }

      // --- Unknown flag ---
    } else {
      std::cerr << "Error: Unknown command or flag: " << command << std::endl;
      print_usage(argv[0]);
      return 1;
    }
  }  // end while loop

  if (!processed_at_least_one) {
    std::cerr << "Error: No valid commands were processed." << std::endl;
    print_usage(argv[0]);
    return 1;
  } else
    return 0;
}

void waybar_local_metrics(const std::string& config_file) {
  CombinedMetrics metrics;
  get_local_metrics(config_file, metrics);
  generate_waybar_output(metrics);
}
void waybar_server_metrics(const std::string& config_file) {
  CombinedMetrics metrics;
  get_server_metrics(config_file, metrics);
  generate_waybar_output(metrics);
}
void waybar_server_metrics(const std::string& config_file,
                           const std::string& host, const std::string& user) {
  CombinedMetrics metrics;
  get_server_metrics(config_file, metrics, host, user);
  generate_waybar_output(metrics);
}
