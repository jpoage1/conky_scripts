// include/metric_settings.hpp
#ifndef METRIC_SETTINGS_HPP
#define METRIC_SETTINGS_HPP

#include "batteryinfo.hpp"

struct MetricSettings {
  bool enable_sysinfo = true;
  bool enable_load_and_process_stats = true;
  bool enable_uptime = true;
  bool enable_memory = true;
  bool enable_cpu_temp = true;
  bool enable_battery_info = true;
  bool enable_stability_info = true;

  bool enable_cpuinfo = true;
  bool enable_network_stats = true;
  bool enable_diskstat = true;

  bool enable_avg_processinfo_cpu = true;
  bool enable_avg_processinfo_mem = true;
  bool enable_realtime_processinfo_cpu = true;
  bool enable_realtime_processinfo_mem = true;
  long unsigned int process_count = true;
  bool only_user_processes = false;
  std::vector<BatteryConfig> batteries;

  std::vector<std::string> ignore_list;

  bool enable_processinfo() {
    return enable_avg_processinfo_cpu || enable_avg_processinfo_mem ||
           enable_realtime_processinfo_cpu || enable_realtime_processinfo_mem;
  }
};
#endif
