// json_definitions.hpp
#pragma once

// 1. Include all struct definitions FIRST
#include "corestat.h"
#include "data.h"
#include "networkstats.hpp"
#include "processinfo.hpp"
#include "runner.hpp"

// 2. Include nlohmann/json.hpp SECOND
#include "nlohmann/json.hpp"

// Use the nlohmann namespace for convenience
using json = nlohmann::json;

// --- Define serialization for each struct ---

// 3. Explicit inline functions for ProcessInfo
// These must be defined *after* nlohmann/json.hpp is included
inline void to_json(json& j, const ProcessInfo& p) {
  j = json{{"pid", p.pid},
           {"vmRssKb", p.vmRssKb},
           {"cpu_percent", p.cpu_percent},
           {"mem_percent", p.mem_percent},
           {"name", p.name}};
}

inline void from_json(const json& j, ProcessInfo& p) {
  j.at("pid").get_to(p.pid);
  j.at("vmRssKb").get_to(p.vmRssKb);
  j.at("cpu_percent").get_to(p.cpu_percent);
  j.at("mem_percent").get_to(p.mem_percent);
  j.at("name").get_to(p.name);
}

// 4. All other macros
// DeviceInfo
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DeviceInfo, device_path, mount_point,
                                   used_bytes, size_bytes);

// CoreStats
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CoreStats, core_id, user_percent,
                                   nice_percent, system_percent, iowait_percent,
                                   idle_percent, total_usage_percent);

// NetworkInterfaceStats
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NetworkInterfaceStats, interface_name,
                                   rx_bytes_per_sec, tx_bytes_per_sec);

// DiskIoStats
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DiskIoStats, device_name, read_bytes_per_sec,
                                   write_bytes_per_sec);
// SystemMetrics
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    SystemMetrics, cores, cpu_frequency_ghz, cpu_temp_c, mem_used_kb,
    mem_total_kb, mem_percent, swap_used_kb, swap_total_kb, swap_percent,
    uptime, load_avg_1m, load_avg_5m, load_avg_15m, processes_total,
    processes_running, sys_name, node_name, kernel_release, machine_type,
    network_interfaces, top_processes_avg_mem, top_processes_avg_cpu,
    top_processes_real_mem, top_processes_real_cpu, disk_io_rates);

// CombinedMetrics
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CombinedMetrics, system, disks);

// MetricsContext
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MetricsContext, source_name, device_file,
                                   metrics, error_message, success,
                                   specific_interfaces);
