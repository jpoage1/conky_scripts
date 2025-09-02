#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "data.h"
#include "data_local.h"
#include "data_ssh.h"
#include "device_info.h"
#include "diskstat.h"
#include "nlohmann/json.hpp"  // The JSON library
#include "size_format.h"
#include "ssh.h"
#include "types.h"

// Use the nlohmann namespace
using json = nlohmann::json;

struct CombinedMetrics {
  SystemMetrics system;
  std::vector<DeviceInfo> disks;
};

int get_metrics(const std::string& config_file, const bool use_ssh,
                CombinedMetrics& metrics) {
  LocalDataStreams local_streams;
  ProcDataStreams ssh_streams;
  DataStreamProvider* provider = nullptr;

  if (use_ssh) {
    if (setup_ssh_session() != 0) {
      std::cerr << "Failed to set up SSH session. Exiting." << std::endl;
      return 2;
    }
    ssh_streams = get_ssh_streams();
    provider = &ssh_streams;
  } else {
    local_streams = get_local_file_streams();
    provider = &local_streams;
  }
  std::vector<std::string> device_paths;
  if (read_device_paths(config_file, device_paths) == 1) {
    std::cout << "${color red}Unable to load file: " + config_file + "${color}"
              << std::endl;
    return 1;
  }

  if (provider) {
    metrics.system = read_data(*provider);
    metrics.disks = collect_device_info(*provider, device_paths);
  } else {
    std::cerr << "Error: No data provider could be initialized." << std::endl;
    return 1;
  }
  if (use_ssh) {
    cleanup_ssh_session();
  }
  return 0;
}
//================================================================================
// WAYBAR OUTPUT GENERATOR (Unchanged)
//================================================================================
void generate_waybar_output(const CombinedMetrics& all_metrics) {
  const auto& metrics = all_metrics.system;
  const auto& devices = all_metrics.disks;
  const TargetFormat target = TargetFormat::WAYBAR;  // Define our target

  json waybar_output;

  // --- Main Text ---
  std::stringstream text_ss;
  text_ss << " " << "N/A" /* TODO: CPU % */ << "% | "
          << " " << metrics.mem_percent << "%";
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
  tooltip_ss << std::left << std::setw(15) << "Mount" << std::setw(25) << "Used"
             << std::setw(25) << "Total" << "\n";

  for (const auto& dev : devices) {
    uint64_t used_percent =
        (dev.size_bytes == 0) ? 0 : (dev.used_bytes * 100) / dev.size_bytes;
    tooltip_ss << std::left << std::setw(15) << dev.mount_point << std::setw(25)
               << format_size(dev.used_bytes).formatted(target) << std::setw(25)
               << format_size(dev.size_bytes).formatted(target) << "("
               << used_percent << "%)\n";
  }
  waybar_output["tooltip"] = tooltip_ss.str();

  // --- CSS Class ---
  if (metrics.mem_percent > 80) {
    waybar_output["class"] = "critical";
  } else if (metrics.mem_percent > 60) {
    waybar_output["class"] = "warning";
  }

  std::cout << waybar_output.dump() << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <device_list_file>\n";
    return 1;
  }

  // 1. Gather all your data into the new combined struct
  CombinedMetrics metrics;
  get_metrics(argv[1], true, metrics);
  generate_waybar_output(metrics);
  return 0;
}
