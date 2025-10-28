#pragma once

#include "corestat.h"        // Includes CoreStats
#include "networkstats.hpp"  // Includes NetworkInterfaceStats
#include "nlohmann/json.hpp"
#include "waybar_types.h"  // Includes MetricResult, CombinedMetrics, etc.
// Add other headers defining your structs if they aren't included via
// waybar_types.h

// Use the nlohmann namespace for convenience
using json = nlohmann::json;

// --- Define serialization for each struct ---

// Example for DeviceInfo (adjust field names if they differ)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DeviceInfo, device_path, mount_point,
                                   used_bytes, size_bytes, used_space, size,
                                   used_space_percent, read_bytes_per_sec,
                                   write_bytes_per_sec);

// Example for CoreStats
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CoreStats, core_id, user_percent,
                                   nice_percent, system_percent, iowait_percent,
                                   idle_percent, total_usage_percent);

// Example for NetworkInterfaceStats
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NetworkInterfaceStats, interface_name,
                                   rx_bytes_per_sec, tx_bytes_per_sec);

// Example for SystemMetrics
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SystemMetrics, cores, cpu_frequency_ghz,
                                   cpu_temp_c, mem_used_kb, mem_total_kb,
                                   mem_percent, swap_used_kb, swap_total_kb,
                                   swap_percent, uptime, load_avg_1m,
                                   load_avg_5m, load_avg_15m, processes_total,
                                   processes_running, sys_name, node_name,
                                   kernel_release, machine_type,
                                   network_interfaces);

// Example for CombinedMetrics
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CombinedMetrics, system, disks);

// Example for MetricResult
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MetricResult, source_name, device_file,
                                   metrics, error_message, success,
                                   specific_interfaces);

// --- End Definitions ---
