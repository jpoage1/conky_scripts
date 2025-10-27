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

  std::string formatted(TargetFormat target) const {
    if (target == TargetFormat::WAYBAR) {
      return "<span foreground='#" + color + "'>" + text + "</span>";
    }

    return text;
  }
};
struct MetricResult {
  std::string source_name;
  CombinedMetrics metrics;    // Will be empty on error
  std::string error_message;  // Will be empty on success
  bool success;
};

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
            << "Uptime: " << system_metrics.uptime << "\n"
            << "CPU Freq: " << std::fixed << std::setprecision(2)
            << system_metrics.cpu_frequency_ghz << " GHz\n"
            << "CPU Temp: " << std::fixed << std::setprecision(2)
            << system_metrics.cpu_temp_c << " C\n"
            << "Memory: "
            << format_size(system_metrics.mem_used_kb * 1024).formatted(target)
            << " / "
            << format_size(system_metrics.mem_total_kb * 1024).formatted(target)
            << " (" << system_metrics.mem_percent << "%)\n";

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

          tooltip_ss << "  " << std::setw(4) << std::left
                     << label  // Use the new label
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
      }
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
int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  std::vector<std::string> args(argv, argv + argc);

  // This vector will hold all results.
  std::vector<MetricResult> all_results;

  long unsigned int i = 1;
  while (i < args.size()) {
    std::string command = args[i];
    std::string config_file;

    // --- Check for Implicit Local ---
    if (i == 1 && command.rfind("--", 0) != 0) {
      MetricResult result;
      result.source_name = "Local";
      config_file = command;
      if (check_config_file(config_file) == 0) {
        result.success = (get_local_metrics(config_file, result.metrics) == 0);
        if (!result.success)
          result.error_message = "Failed to get local metrics.";
      } else {
        result.success = false;
        result.error_message = "Config file not found: " + config_file;
      }
      all_results.push_back(result);
      i += 1;

      // --- Check for Explicit --local ---
    } else if (command == "--local") {
      MetricResult result;
      result.source_name = "Local";
      if (i + 1 >= args.size()) {
        result.success = false;
        result.error_message = "--local requires a <config_file> argument.";
        i += 1;
      } else {
        config_file = args[i + 1];
        if (check_config_file(config_file) == 0) {
          result.success =
              (get_local_metrics(config_file, result.metrics) == 0);
          if (!result.success)
            result.error_message = "Failed to get local metrics.";
        } else {
          result.success = false;
          result.error_message = "Config file not found: " + config_file;
        }
        i += 2;
      }
      all_results.push_back(result);

      // --- Check for --ssh ---
    } else if (command == "--ssh") {
      MetricResult result;
      if (i + 1 >= args.size()) {
        result.success = false;
        result.error_message = "--ssh requires a <config_file> argument.";
        i += 1;
      } else {
        config_file = args[i + 1];
        // Custom SSH
        if (i + 3 < args.size() && args[i + 2].rfind("--", 0) != 0) {
          std::string host = args[i + 2];
          std::string user = args[i + 3];
          result.source_name = user + "@" + host;
          if (check_config_file(config_file) == 0) {
            result.success = (get_server_metrics(config_file, result.metrics,
                                                 host, user) == 0);
            if (!result.success) result.error_message = "Failed to connect.";
          } else {
            result.success = false;
            result.error_message = "Config file not found: " + config_file;
          }
          i += 4;
        } else {
          // Default SSH
          result.source_name = "Default SSH";
          if (check_config_file(config_file) == 0) {
            result.success =
                (get_server_metrics(config_file, result.metrics) == 0);
            if (!result.success) result.error_message = "Failed to connect.";
          } else {
            result.success = false;
            result.error_message = "Config file not found: " + config_file;
          }
          i += 2;
        }
      }
      all_results.push_back(result);

      // --- Unknown flag ---
    } else {
      MetricResult result;
      result.source_name = "Parser";
      result.success = false;
      result.error_message = "Unknown command or flag: " + command;
      all_results.push_back(result);
      i += 1;
    }
  }  // end while loop

  if (all_results.empty()) {
    std::cerr << "Error: No commands were processed." << std::endl;
    print_usage(argv[0]);
    return 1;
  }

  // Call the single output function ONCE.
  generate_waybar_output(all_results);
  return 0;
}
