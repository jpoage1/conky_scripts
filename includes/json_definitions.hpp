// json_definitions.hpp
#ifndef JSON_DEFINITIONS_HPP
#define JSON_DEFINITIONS_HPP
#include <nlohmann/json_fwd.hpp>

struct DeviceInfo;
struct BatteryStatus;
struct Time;
struct DiskUsage;
struct DiskIoStats;
struct HdIoStats;
struct CoreStats;
struct NetworkInterfaceStats;
struct MemInfo;
struct ProcessInfo;
struct SystemStability;
struct Time;

class SystemMetrics;

// Use the nlohmann namespace for convenience
using json = nlohmann::json;

// Battery
void to_json(json &j, const BatteryStatus &s);
void from_json(const json &j, BatteryStatus &s);

// Time (Uptime)
void to_json(json &j, const Time &s);
void from_json(const json &j, Time &s);

// Storage
void to_json(json &j, const DiskUsage &s);
void from_json(const json &j, DiskUsage &s);

void to_json(json &j, const DiskIoStats &s);
void from_json(const json &j, DiskIoStats &s);

void to_json(json &j, const DeviceInfo &s);
void from_json(const json &j, DeviceInfo &s);

void to_json(json &j, const HdIoStats &s);
void from_json(const json &j, HdIoStats &s);

// CPU
void to_json(json &j, const CoreStats &s);
void from_json(const json &j, CoreStats &s);

// Network
void to_json(json &j, const NetworkInterfaceStats &s);
void from_json(const json &j, NetworkInterfaceStats &s);

// Memory
void to_json(json &j, const MemInfo &s);
void from_json(const json &j, MemInfo &s);

// ProcessInfo
void to_json(json &j, const ProcessInfo &p);
void from_json(const json &j, ProcessInfo &p);

// Uptime
void to_json(json &j, const Time &t);
void from_json(const json &j, Time &t);

// System Metrics
void to_json(json &j, const SystemMetrics &s);
void from_json(const json &j, SystemMetrics &s);
void from_json(const json &j, SystemStability &s);

// Stability & PSI Metrics
void to_json(json &j, const SystemStability &s);
void from_json(const json &j, SystemStability &s);
#endif
