// waybar_formatters.cpp

#include "nlohmann/json.hpp"
#include "size_format.h"
#include "waybar_cli_parser.hpp"
#include "waybar_formatters.h"
#include "waybar_types.h"

using json = nlohmann::json;

FormattedSize format_size_rate(double bytes_per_sec);
void generate_waybar_output(const std::vector<MetricResult>& all_results);

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

void generate_waybar_output(const std::vector<MetricResult>& all_results) {
  json waybar_output;
  std::stringstream tooltip_ss;
  bool any_errors = false;
  int total_mem_percent = 0;
  int valid_mem_sources = 0;

  //   const TargetFormat target = TargetFormat::WAYBAR;

  for (const auto& result : all_results) {
    if (result.success) {
      const auto& system_metrics = result.metrics.system;
      const auto& devices = result.metrics.disks;
      tooltip_ss << show_system_metrics(result, system_metrics,
                                        total_mem_percent, valid_mem_sources)

                 << show_top_mem_procs(result, system_metrics.top_processes_mem)
                 << show_top_cpu_procs(result, system_metrics.top_processes_cpu)
                 << show_devices(result, devices)

                 << show_network_interfaces(result,
                                            system_metrics.network_interfaces,
                                            result.specific_interfaces);

    } else {
      // --- This result was an ERROR, print the error message ---
      any_errors = true;
      tooltip_ss
          << "<b><span foreground='#f38ba8'>⚠ Error: "  // Use a standard icon
          << result.source_name << "</span></b>\n"
          << "<tt>  " << result.error_message << "</tt>\n\n";  // Indent message
    }
  }  // end for loop

  std::string main_text_label = " Unknown";  // Default
  if (!all_results.empty()) {
    const auto& first_result =
        all_results[0];  // Assume only one result due to toggle logic
    main_text_label = " " + first_result.source_name;
    if (!first_result.success) {
      any_errors = true;
      main_text_label += " ⚠";  // Append warning icon on error
    }
  } else {
    // Handle case where all_results might be empty (though should not happen
    // with current main logic)
    any_errors = true;  // Treat empty result as an error case for text/class
    main_text_label = " Error";
  }
  waybar_output["text"] = main_text_label;

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

std::string show_top_mem_procs(const MetricResult& result,
                               const std::vector<ProcessInfo>& top_procs_mem) {
  std::stringstream tooltip_ss;
  if (!top_procs_mem.empty()) {
    tooltip_ss << "<b>Top Processes (Mem) (" << result.source_name << ")</b>\n";
    tooltip_ss << "<tt>";

    const size_t pid_col_width = 7;
    const size_t rss_col_width = 12;      // "VmRSS (MiB)"
    const size_t mem_perc_col_width = 6;  // "%Mem"
    std::string pid_header = "PID";
    std::string rss_header = "VmRSS (MiB)";
    std::string mem_perc_header = "%Mem";
    std::string name_header = "Name";

    // Set precision for this block
    tooltip_ss << std::fixed << std::setprecision(1);

    tooltip_ss << std::left << std::setw(pid_col_width) << pid_header
               << std::right << std::setw(rss_col_width) << rss_header
               << std::right << std::setw(mem_perc_col_width) << mem_perc_header
               << "  " << name_header << "\n";

    for (const auto& proc : top_procs_mem) {
      double vmRssMiB = static_cast<double>(proc.vmRssKb) / 1024.0;
      tooltip_ss << std::left << std::setw(pid_col_width) << proc.pid
                 << std::right << std::setw(rss_col_width)
                 << vmRssMiB
                 // Add Mem % column
                 << std::right << std::setw(mem_perc_col_width - 1)
                 << proc.mem_percent << "%"
                 << "  " << proc.name << "\n";
    }
    tooltip_ss << "</tt>\n\n";
  }
  return tooltip_ss.str();
}

std::string show_top_cpu_procs(const MetricResult& result,
                               const std::vector<ProcessInfo>& top_procs_cpu) {
  std::stringstream tooltip_ss;
  if (!top_procs_cpu.empty()) {
    tooltip_ss << "<b>Top Processes (CPU) (" << result.source_name << ")</b>\n";
    tooltip_ss << "<tt>";

    const size_t pid_col_width = 7;
    const size_t cpu_col_width = 6;       // Width for "  %CPU"
    const size_t mem_perc_col_width = 6;  // Width for "  %Mem"
    std::string pid_header = "PID";
    std::string cpu_header = "%CPU";
    std::string mem_perc_header = "%Mem";
    std::string name_header = "Name";

    tooltip_ss << std::fixed << std::setprecision(1);

    // Assumes std::fixed and std::setprecision(1) are already set
    tooltip_ss << std::left << std::setw(pid_col_width) << pid_header
               << std::right << std::setw(cpu_col_width) << cpu_header
               << std::right << std::setw(mem_perc_col_width) << mem_perc_header
               << "  " << name_header << "\n";

    for (const auto& proc : top_procs_cpu) {
      tooltip_ss << std::left << std::setw(pid_col_width)
                 << proc.pid
                 // CPU % column
                 << std::right << std::setw(cpu_col_width - 1)
                 << proc.cpu_percent
                 << "%"
                 // Add Mem % column
                 << std::right << std::setw(mem_perc_col_width - 1)
                 << proc.mem_percent << "%"
                 << "  " << proc.name << "\n";
    }
    tooltip_ss << "</tt>\n\n";
  }
  return tooltip_ss.str();
}

std::string show_network_interfaces(
    const MetricResult& result,
    const std::vector<NetworkInterfaceStats>& network_interfaces,
    const std::set<std::string>& specific_interfaces) {
  std::stringstream tooltip_ss;
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

      // Rows - Iterate over the filtered list
      for (const auto& net_stat : filtered_interfaces) {
        // Get the formatted size objects (containing text and color)
        auto down_formatted_obj = format_size_rate(net_stat.rx_bytes_per_sec);
        auto up_formatted_obj = format_size_rate(net_stat.tx_bytes_per_sec);

        // Create stringstreams and pad the RAW text
        std::stringstream down_text_ss;
        down_text_ss << std::left
                     << std::setw(rate_col_width - 1)  // Leave 1 space padding
                     << down_formatted_obj.text;

        std::stringstream up_text_ss;
        up_text_ss << std::left
                   << std::setw(rate_col_width - 1)  // Leave 1 space padding
                   << up_formatted_obj.text;

        // Construct the table row with manual Pango spans
        tooltip_ss << std::left << std::setw(if_col_width)
                   << net_stat.interface_name
                   // Down rate: wrap padded text in span
                   << "<span foreground='#" << down_formatted_obj.color << "'>"
                   << down_text_ss.str()
                   << "</span> "  // Add manual space
                   // Up rate: wrap padded text in span
                   << "<span foreground='#" << up_formatted_obj.color << "'>"
                   << up_text_ss.str()
                   << "</span>"  // No trailing space before newline
                   << "\n";
      }
    }
    tooltip_ss << "</tt>\n\n";
  }
  return tooltip_ss.str();
}

std::string show_devices(const MetricResult& result,
                         const std::vector<DeviceInfo>& devices) {
  std::stringstream tooltip_ss;
  if (!devices.empty()) {
    tooltip_ss << "<b>Filesystem Usage (" << result.source_name << ")</b>\n";
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
               << std::setw(data_col_width + 1) << total_header << " Used%\n";

    for (const auto& dev : devices) {
      if (dev.mount_point.empty()) continue;
      uint64_t used_percent =
          (dev.size_bytes == 0) ? 0 : (dev.used_bytes * 100) / dev.size_bytes;
      auto fs_used = format_size(dev.used_bytes);
      auto fs_total = format_size(dev.size_bytes);
      std::stringstream used_text_ss;
      used_text_ss << std::left << std::setw(data_col_width) << fs_used.text;
      std::stringstream total_text_ss;
      total_text_ss << std::left << std::setw(data_col_width) << fs_total.text;

      tooltip_ss << std::left << std::setw(mount_col_width) << dev.mount_point
                 << "<span foreground='#" << fs_used.color << "'>"
                 << used_text_ss.str() << "</span> "
                 << "<span foreground='#" << fs_total.color << "'>"
                 << total_text_ss.str() << "</span> "
                 << "(" << used_percent << "%)\n";
    }
    tooltip_ss << "</tt>\n\n";
  }
  return tooltip_ss.str();
}

std::string show_system_metrics(const MetricResult& result,
                                const SystemMetrics& system_metrics,
                                int& total_mem_percent,
                                int& valid_mem_sources) {
  const TargetFormat target = TargetFormat::WAYBAR;
  std::stringstream tooltip_ss;
  if (system_metrics.mem_total_kb > 0) {
    tooltip_ss
        << "<b>System Information (" << result.source_name << ")</b>\n"
        << "<tt>"  // Use tt for consistent spacing if needed
        << system_metrics.sys_name << " " << system_metrics.node_name << " "
        << system_metrics.kernel_release << " " << system_metrics.machine_type
        << "</tt>\n"  // End tt tag
        << "Uptime: " << system_metrics.uptime << "\n"
        << "Load (1m/5m/15m): " << std::fixed << std::setprecision(2)
        << system_metrics.load_avg_1m << " / " << system_metrics.load_avg_5m
        << " / " << system_metrics.load_avg_15m << "\n"
        << "Processes: " << system_metrics.processes_running << " running / "
        << system_metrics.processes_total << " total\n"

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
        << format_size(system_metrics.swap_total_kb * 1024).formatted(target)
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

      tooltip_ss << "  " << std::setw(4) << std::left << label << std::setw(7)
                 << std::right << core.total_usage_percent << "%"
                 << std::setw(7) << std::right << core.user_percent << "%"
                 << std::setw(7) << std::right << core.system_percent << "%"
                 << std::setw(7) << std::right << core.iowait_percent << "%"
                 << "\n";
    }
    tooltip_ss << "</tt>\n\n";

    total_mem_percent += system_metrics.mem_percent;
    valid_mem_sources++;
  }
  return tooltip_ss.str();
}
