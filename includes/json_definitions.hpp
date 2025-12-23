// json_definitions.hpp
#ifndef JSON_DEFINITIONS_HPP
#define JSON_DEFINITIONS_HPP

#include "batteryinfo.hpp"
#include "corestat.hpp"
#include "diskstat.hpp"
#include "filesystems.hpp"
#include "log.hpp"
#include "meminfo.hpp"
#include "metrics.hpp"
#include "networkstats.hpp"
#include "nlohmann/json.hpp"
#include "processinfo.hpp"
#include "stream_provider.hpp"
#include "uptime.hpp"

// Use the nlohmann namespace for convenience
using json = nlohmann::json;

// ProcessInfo
void to_json(json& j, const ProcessInfo& p);

void from_json(const json& j, ProcessInfo& p);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BatteryStatus, name, percentage, status);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Time, days, hours, minutes, seconds);

// DeviceInfo
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DiskUsage, used_bytes, size_bytes);

// DiskIOStats
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DiskIoStats, read_bytes_per_sec,
                                   write_bytes_per_sec);

// DeviceInfo
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DeviceInfo, device_path, mount_point, usage,
                                   io);

// CoreStats
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CoreStats, core_id, user_percent,
                                   nice_percent, system_percent, iowait_percent,
                                   idle_percent, total_usage_percent);

// NetworkInterfaceStats
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NetworkInterfaceStats, interface_name,
                                   rx_bytes_per_sec, tx_bytes_per_sec);

// HdIoStats
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HdIoStats, device_name, read_bytes_per_sec,
                                   write_bytes_per_sec);

// MemInfo
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MemInfo, used_kb, total_kb, percent);

void to_json(json& j, const Time& t);

void from_json(const json& j, Time& t);
// 4. Define serialization for the PARENT STRUCT (SystemMetrics) LAST
//    Now it can find the definitions for all its members.
void to_json(json& j, const SystemMetrics& s);

void from_json(const json& j, SystemMetrics& s);

#endif
