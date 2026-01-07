#ifndef METRIC_SETTINGS_HPP
#define METRIC_SETTINGS_HPP

#include "batteryinfo.hpp"
#include "data_ssh.hpp"
#include "diskstat.hpp"
#include "processinfo.hpp"
#include "provider.hpp"
#include "window_settings.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace telemetry {
struct WindowConfig;

namespace fs = std::filesystem;
// Features
struct Features {
  bool enable_sysinfo = true;
  bool enable_memory = true;
  bool enable_cpuinfo = true;
  bool enable_cpu_temp = true;
  bool enable_uptime = true;
  bool enable_load_and_process_stats = true;
  bool enable_diskstat = true;
  bool enable_network_stats = true;
  Processes processes;

}; // End Features struct

struct MetricSettings {
  std::string name;
  Features features;
  Batteries batteries;
  Storage storage;

  std::string stream_provider;
  ProviderSettings provider_settings;

  SSH ssh;

  WindowConfig window;

  /* LEGACY LOGIC */
  // Core Features
  bool enable_uptime = true;
  bool enable_sysinfo = true;
  bool enable_memory = true;

  // Stats
  bool enable_load_and_process_stats = true;
  bool enable_cpu_temp = true;

  // Stability
  bool enable_stability_info = true;

  bool enable_cpuinfo = true;
  bool enable_network_stats = true;
  bool enable_diskstat = true;

  // ProcessInfo
  bool enable_avg_processinfo_cpu = true;
  bool enable_avg_processinfo_mem = true;
  bool enable_realtime_processinfo_cpu = true;
  bool enable_realtime_processinfo_mem = true;
  long unsigned int process_count = true;
  bool only_user_processes = false;

  // Batteries
  bool enable_battery_info = true;

  std::vector<std::string> ignore_list;
  std::set<std::string> interfaces;
  std::vector<std::string> filesystems;

  bool enable_processinfo() const {
    return enable_avg_processinfo_cpu || enable_avg_processinfo_mem ||
           enable_realtime_processinfo_cpu || enable_realtime_processinfo_mem;
  }
}; // End MetricSettings struct

struct MetricsConfig {

  std::string run_mode = "persistent";
  std::string output_format = "json";
  int polling_interval_ms = 1000;
  std::string log_level = "warn";
  bool dump_to_file = false;
  std::string log_file_path = "/tmp/telemetery.log";
  std::vector<MetricSettings> settings;
}; // End MetricsConfig struct
}; // namespace telemetry
#endif
