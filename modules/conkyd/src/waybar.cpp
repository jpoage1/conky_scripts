#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>
#include <set>
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

  std::string formatted(TargetFormat target) const {
    if (target == TargetFormat::WAYBAR) {
      return "<span foreground='#" + color + "'>" + text + "</span>";
    }

    return text;
  }
};
FormattedSize format_size_rate(double bytes_per_sec);
struct MetricResult {
  std::string source_name;
  std::string device_file;
  CombinedMetrics metrics;    // Will be empty on error
  std::string error_message;  // Will be empty on success
  bool success;
  std::set<std::string> specific_interfaces;
};
std::set<std::string> parse_interface_list(const std::string& list_str) {
  std::set<std::string> interfaces;
  std::stringstream ss(list_str);
  std::string interface_name;
  while (std::getline(ss, interface_name, ',')) {
    if (!interface_name.empty()) {
      interfaces.insert(interface_name);
    }
  }
  return interfaces;
}

void generate_waybar_output(const std::vector<MetricResult>& all_results) {
  json waybar_output;
  std::stringstream tooltip_ss;
  bool any_errors = false;
  int total_mem_percent = 0;
  int valid_mem_sources = 0;

  const TargetFormat target = TargetFormat::WAYBAR;

  for (const auto& result : all_results) {
    if (result.success) {
      const auto& system_metrics = result.metrics.system;
      const auto& devices = result.metrics.disks;
      if (system_metrics.mem_total_kb > 0) {
        tooltip_ss
            << "<b>System Information (" << result.source_name << ")</b>\n"
            << "<tt>"  // Use tt for consistent spacing if needed
            << system_metrics.sys_name << " " << system_metrics.node_name << " "
            << system_metrics.kernel_release << " "
            << system_metrics.machine_type << "</tt>\n"  // End tt tag
            << "Uptime: " << system_metrics.uptime << "\n"
            << "Load (1m/5m/15m): " << std::fixed << std::setprecision(2)
            << system_metrics.load_avg_1m << " / " << system_metrics.load_avg_5m
            << " / " << system_metrics.load_avg_15m << "\n"
            << "Processes: " << system_metrics.processes_running
            << " running / " << system_metrics.processes_total << " total\n"

            << "CPU Freq: " << std::fixed << std::setprecision(2)
            << system_metrics.cpu_frequency_ghz << " GHz\n"
            << "CPU Temp: " << std::fixed << std::setprecision(2)
            << system_metrics.cpu_temp_c << " C\n"
            << "Memory: "
            << format_size(system_metrics.mem_used_kb * 1024).formatted(target)
            << " / "
            << format_size(system_metrics.mem_total_kb * 1024).formatted(target)
            << " (" << system_metrics.mem_percent << "%)\n"
            << "Swap: "
            << format_size(system_metrics.swap_used_kb * 1024).formatted(target)
            << " / "
            << format_size(system_metrics.swap_total_kb * 1024)
                   .formatted(target)
            << " (" << system_metrics.swap_percent << "%)\n";
        tooltip_ss << "<tt>";

        tooltip_ss << std::fixed << std::setprecision(1);
        tooltip_ss << "  Core   Total   User    Sys   IOWait\n";

        for (const auto& core : system_metrics.cores) {
          std::string label;
          if (core.core_id == 0) {
            label = "Avg";
          } else {
            label = std::to_string(core.core_id - 1);
          }

          tooltip_ss << "  " << std::setw(4) << std::left << label
                     << std::setw(7) << std::right << core.total_usage_percent
                     << "%" << std::setw(7) << std::right << core.user_percent
                     << "%" << std::setw(7) << std::right << core.system_percent
                     << "%" << std::setw(7) << std::right << core.iowait_percent
                     << "%"
                     << "\n";
        }
        tooltip_ss << "</tt>\n\n";

        total_mem_percent += system_metrics.mem_percent;
        valid_mem_sources++;
      }
      if (!devices.empty()) {
        tooltip_ss << "<b>Filesystem Usage (" << result.source_name
                   << ")</b>\n";
        tooltip_ss << "<tt>";

        size_t max_mount_len = 5;
        for (const auto& dev : devices) {
          if (dev.mount_point.length() > max_mount_len) {
            max_mount_len = dev.mount_point.length();
          }
        }
        const size_t mount_col_width = max_mount_len + 1;
        const size_t data_col_width = 9;
        const std::string used_header = "Used";
        const std::string total_header = "Total";

        tooltip_ss << std::left << std::setw(mount_col_width) << "Mount"
                   << std::setw(data_col_width + 1) << used_header
                   << std::setw(data_col_width + 1) << total_header
                   << " Used%\n";

        for (const auto& dev : devices) {
          if (dev.mount_point.empty()) continue;
          uint64_t used_percent = (dev.size_bytes == 0)
                                      ? 0
                                      : (dev.used_bytes * 100) / dev.size_bytes;
          auto fs_used = format_size(dev.used_bytes);
          auto fs_total = format_size(dev.size_bytes);
          std::stringstream used_text_ss;
          used_text_ss << std::left << std::setw(data_col_width)
                       << fs_used.text;
          std::stringstream total_text_ss;
          total_text_ss << std::left << std::setw(data_col_width)
                        << fs_total.text;

          tooltip_ss << std::left << std::setw(mount_col_width)
                     << dev.mount_point << "<span foreground='#"
                     << fs_used.color << "'>" << used_text_ss.str()
                     << "</span> "
                     << "<span foreground='#" << fs_total.color << "'>"
                     << total_text_ss.str() << "</span> "
                     << "(" << used_percent << "%)\n";
        }
        tooltip_ss << "</tt>\n\n";
      }  // Get the network interfaces for *this* result
      const auto& network_interfaces = result.metrics.system.network_interfaces;
      // Get the specific interfaces requested for *this* command
      const auto& specific_interfaces = result.specific_interfaces;

      if (!network_interfaces.empty()) {
        tooltip_ss << "<b>Network Usage (" << result.source_name << ")</b>\n";
        tooltip_ss << "<tt>";

        // --- FILTERING STEP ---
        std::vector<NetworkInterfaceStats> filtered_interfaces;
        if (!specific_interfaces.empty()) {
          for (const auto& net_stat : network_interfaces) {
            if (specific_interfaces.count(net_stat.interface_name)) {
              filtered_interfaces.push_back(net_stat);
            }
          }
        }
        // --- END FILTERING ---

        if (!filtered_interfaces.empty()) {
          size_t max_if_len = 6;  // "Device"
          for (const auto& net_stat : filtered_interfaces) {
            if (net_stat.interface_name.length() > max_if_len) {
              max_if_len = net_stat.interface_name.length();
            }
          }
          const size_t if_col_width = max_if_len + 1;
          const size_t rate_col_width = 12;

          tooltip_ss << std::left << std::setw(if_col_width) << "Device"
                     << std::setw(rate_col_width) << "Down"
                     << std::setw(rate_col_width) << "Up" << "\n";

          for (const auto& net_stat : filtered_interfaces) {
            auto down_formatted = format_size_rate(net_stat.rx_bytes_per_sec);
            auto up_formatted = format_size_rate(net_stat.tx_bytes_per_sec);

            tooltip_ss << std::left << std::setw(if_col_width)
                       << net_stat.interface_name << std::setw(rate_col_width)
                       << down_formatted.formatted(target)
                       << std::setw(rate_col_width)
                       << up_formatted.formatted(target) << "\n";
          }
        }
        tooltip_ss << "</tt>\n\n";
      }
      // --- END NETWORK BLOCK ---
    } else {
      // --- This result was an ERROR, print the error message ---
      any_errors = true;
      tooltip_ss
          << "<b><span foreground='#f38ba8'>⚠ Error: "  // Use a standard icon
          << result.source_name << "</span></b>\n"
          << "<tt>  " << result.error_message << "</tt>\n\n";  // Indent message
    }
  }  // end for loop

  // --- Set Main Text ---
  if (any_errors) {
    waybar_output["text"] = " ⚠";
  } else {
    waybar_output["text"] = " Stats";
  }

  // --- Set CSS Class ---
  int avg_mem =
      (valid_mem_sources > 0) ? (total_mem_percent / valid_mem_sources) : 0;
  if (any_errors || avg_mem > 80) {
    waybar_output["class"] = "critical";
  } else if (avg_mem > 60) {
    waybar_output["class"] = "warning";
  }

  waybar_output["tooltip"] = tooltip_ss.str();
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

// Helper to process a single command instance (--local or --ssh)
// Returns the number of arguments consumed (including the command itself)
int process_command(const std::vector<std::string>& args, size_t& current_index,
                    std::vector<MetricResult>& all_results) {
  MetricResult result;
  std::string command = args[current_index];
  size_t initial_index = current_index;

  // --- 1. Parse Config File ---
  if (current_index + 1 >= args.size()) {
    result.success = false;
    result.error_message = command + " requires a <config_file> argument.";
    result.source_name =
        (command == "--local") ? "Local (Error)" : "SSH (Error)";
    all_results.push_back(result);
    return 1;  // Consume only the command itself
  }
  std::string config_file = args[current_index + 1];
  current_index += 2;  // Tentatively consume command + config

  // --- 2. Check Config File ---
  if (check_config_file(config_file) != 0) {
    result.success = false;
    result.error_message = "Config file not found: " + config_file;
    result.source_name =
        (command == "--local") ? "Local (Error)" : "SSH (Error)";
    all_results.push_back(result);
    return current_index - initial_index;  // Return consumed args
  }

  // --- 3. Call Appropriate Get Function ---
  bool success = false;
  if (command == "--local") {
    result.source_name = "Local";
    success = (get_local_metrics(config_file, result.metrics) == 0);
    if (!success) result.error_message = "Failed to get local metrics.";

  } else if (command == "--ssh") {
    // Check for specific host/user
    if (current_index + 1 < args.size() &&
        args[current_index].rfind("--", 0) != 0 &&
        args[current_index + 1].rfind("--", 0) != 0) {
      std::string host = args[current_index];
      std::string user = args[current_index + 1];
      result.source_name = user + "@" + host;
      success =
          (get_server_metrics(config_file, result.metrics, host, user) == 0);
      if (!success) result.error_message = "Failed to connect.";
      current_index += 2;  // Consume host + user
    } else {
      // Default SSH host
      result.source_name = "Default SSH";
      success = (get_server_metrics(config_file, result.metrics) == 0);
      if (!success) result.error_message = "Failed to connect.";
    }
  }
  // --- 4. Check for --interfaces ---
  if (current_index < args.size() && args[current_index] == "--interfaces") {
    if (current_index + 1 < args.size()) {
      result.specific_interfaces =
          parse_interface_list(args[current_index + 1]);
      current_index += 2;  // Consume --interfaces and its value
    } else {
      // Handle error: --interfaces flag without a value
      // You might want to add an error to result.error_message
      std::cerr << "Warning: --interfaces flag requires a comma-separated list."
                << std::endl;
      current_index += 1;  // Consume only the flag to avoid infinite loop
    }
  }

  result.success = success;
  all_results.push_back(result);
  return current_index - initial_index;  // Return total consumed args
}
int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  std::vector<std::string> args(argv, argv + argc);
  std::vector<MetricResult> all_results;
  size_t i = 1;  // Use size_t for index

  while (i < args.size()) {
    std::string command = args[i];
    int consumed = 0;

    // --- Handle Implicit Local (First argument only) ---
    if (i == 1 && command.rfind("--", 0) != 0) {
      // Temporarily prepend "--local" to simulate the command structure
      std::vector<std::string> temp_args = {"program_name", "--local"};
      temp_args.insert(temp_args.end(), args.begin() + 1, args.end());
      size_t temp_i = 1;  // Start processing at "--local"
      consumed = process_command(temp_args, temp_i, all_results);
      // Adjust consumption: consumed includes the fake "--local", subtract 1
      consumed = (consumed > 0) ? consumed - 1 : 0;

      // --- Handle Explicit Commands ---
    } else if (command == "--local" || command == "--ssh") {
      consumed = process_command(args, i, all_results);

      // --- Handle Unknown ---
    } else {
      MetricResult result;
      result.source_name = "Parser";
      result.success = false;
      result.error_message = "Unknown command or flag: " + command;
      all_results.push_back(result);
      consumed = 1;  // Consume the unknown command
    }

    // Advance index by the number of arguments consumed by the helper
    i += (consumed > 0) ? consumed : 1;  // Ensure progress even on error

  }  // end while loop

  if (all_results.empty()) {
    // Check if any *successful* results were processed.
    bool any_success = false;
    for (const auto& res : all_results) {
      if (res.success) {
        any_success = true;
        break;
      }
    }
    if (!any_success && !all_results.empty()) {
      // Only errors were generated, still output them
    } else if (all_results.empty()) {
      std::cerr << "Error: No valid commands were processed." << std::endl;
      print_usage(argv[0]);
      return 1;
    }
  }

  //   // Define your desired interfaces here or parse them
  //   std::set<std::string> desired_interfaces = {"wlp3s0",  "enp0s25", "tun0",
  //                                               "docker0", "virbr0",
  //                                               "vnet1"};
  generate_waybar_output(all_results);
  return 0;
}
FormattedSize format_size_rate(double bytes_per_sec) {
  // Implement logic similar to format_size, but add "/s" to the text
  // Example: convert bytes/sec to KiB/s, MiB/s, GiB/s
  // Return FormattedSize{ "123.4 MiB/s", some_color };
  // Placeholder:
  std::stringstream ss;
  ss << std::fixed << std::setprecision(1) << (bytes_per_sec / 1024.0 / 1024.0)
     << " MiB/s";
  return {ss.str(), "00ff00"};  // Placeholder color
}
