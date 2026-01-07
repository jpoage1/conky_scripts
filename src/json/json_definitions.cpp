// json_definitions.cpp
#include "json_definitions.hpp"

#include <nlohmann/json.hpp>

#include "batteryinfo.hpp"
#include "corestat.hpp"
#include "diskstat.hpp"
#include "filesystems.hpp"
#include "json_definitions.hpp"
#include "log.hpp"
#include "meminfo.hpp"
#include "metrics.hpp"
#include "networkstats.hpp"
#include "nlohmann/json.hpp"
#include "processinfo.hpp"
#include "stream_provider.hpp"
#include "uptime.hpp"

namespace telemetry {

// Use the nlohmann namespace for convenience
using json = nlohmann::json;

// --- Battery ---
void to_json(json &j, const BatteryStatus &s) {
  j = json{
      {"name", s.name}, {"percentage", s.percentage}, {"status", s.status}};
}
void from_json(const json &j, BatteryStatus &s) {
  j.at("name").get_to(s.name);
  j.at("percentage").get_to(s.percentage);
  j.at("status").get_to(s.status);
}

// --- Time / Uptime ---
void to_json(json &j, const Time &t) {
  j = json{
      {"days", t.days},           {"hours", t.hours}, {"minutes", t.minutes},
      {"seconds", t.seconds},     {"raw", t.raw},     {"text", t.to_str()},
      {"clock", t.to_clock_str()}};
}
void from_json(const json &j, Time &t) {
  j.at("days").get_to(t.days);
  j.at("hours").get_to(t.hours);
  j.at("minutes").get_to(t.minutes);
  j.at("seconds").get_to(t.seconds);
  j.at("raw").get_to(t.raw);
}

// --- Storage ---
void to_json(json &j, const DiskUsage &s) {
  j = json{{"used_bytes", s.used_bytes}, {"size_bytes", s.size_bytes}};
}
void from_json(const json &j, DiskUsage &s) {
  j.at("used_bytes").get_to(s.used_bytes);
  j.at("size_bytes").get_to(s.size_bytes);
}

// --- Disk IO
void to_json(json &j, const DiskIoStats &s) {
  j = json{{"read_bytes_per_sec", s.read_bytes_per_sec},
           {"write_bytes_per_sec", s.write_bytes_per_sec}};
}
void from_json(const json &j, DiskIoStats &s) {
  j.at("read_bytes_per_sec").get_to(s.read_bytes_per_sec);
  j.at("write_bytes_per_sec").get_to(s.write_bytes_per_sec);
}

// --- Disk Info
void to_json(json &j, const DeviceInfo &s) {
  j = json{{"device_path", s.device_path},
           {"mount_point", s.mount_point},
           {"usage", s.usage},
           {"io", s.io}};
}
void from_json(const json &j, DeviceInfo &s) {
  j.at("device_path").get_to(s.device_path);
  j.at("mount_point").get_to(s.mount_point);
  j.at("usage").get_to(s.usage);
  j.at("io").get_to(s.io);
}

// --- HDD IO
void to_json(json &j, const HdIoStats &s) {
  j = json{{"device_name", s.device_name},
           {"read_bytes_per_sec", s.read_bytes_per_sec},
           {"write_bytes_per_sec", s.write_bytes_per_sec}};
}
void from_json(const json &j, HdIoStats &s) {
  j.at("device_name").get_to(s.device_name);
  j.at("read_bytes_per_sec").get_to(s.read_bytes_per_sec);
  j.at("write_bytes_per_sec").get_to(s.write_bytes_per_sec);
}

// --- CPU ---
void to_json(json &j, const CoreStats &s) {
  j = json{{"core_id", s.core_id},
           {"user_percent", s.user_percent},
           {"nice_percent", s.nice_percent},
           {"system_percent", s.system_percent},
           {"iowait_percent", s.iowait_percent},
           {"idle_percent", s.idle_percent},
           {"total_usage_percent", s.total_usage_percent}};
}
void from_json(const json &j, CoreStats &s) {
  j.at("core_id").get_to(s.core_id);
  j.at("user_percent").get_to(s.user_percent);
  j.at("nice_percent").get_to(s.nice_percent);
  j.at("system_percent").get_to(s.system_percent);
  j.at("iowait_percent").get_to(s.iowait_percent);
  j.at("idle_percent").get_to(s.idle_percent);
  j.at("total_usage_percent").get_to(s.total_usage_percent);
}

// --- Network ---
void to_json(json &j, const NetworkInterfaceStats &s) {
  j = json{{"interface_name", s.interface_name},
           {"rx_bytes_per_sec", s.rx_bytes_per_sec},
           {"tx_bytes_per_sec", s.tx_bytes_per_sec}};
}
void from_json(const json &j, NetworkInterfaceStats &s) {
  j.at("interface_name").get_to(s.interface_name);
  j.at("rx_bytes_per_sec").get_to(s.rx_bytes_per_sec);
  j.at("tx_bytes_per_sec").get_to(s.tx_bytes_per_sec);
}

// --- Memory ---
void to_json(json &j, const MemInfo &s) {
  j = json{
      {"used_kb", s.used_kb}, {"total_kb", s.total_kb}, {"percent", s.percent}};
}
void from_json(const json &j, MemInfo &s) {
  j.at("used_kb").get_to(s.used_kb);
  j.at("total_kb").get_to(s.total_kb);
  j.at("percent").get_to(s.percent);
}

// --- ProcessInfo ---
void to_json(json &j, const ProcessInfo &p) {
  j = json{{"pid", p.pid},
           {"vmRssKb", p.vmRssKb},
           {"cpu_percent", p.cpu_percent},
           {"mem_percent", p.mem_percent},
           {"name", p.name},
           {"open_fds", p.open_fds},
           {"io_read_bytes", p.io_read_bytes},
           {"io_write_bytes", p.io_write_bytes}};
}
void from_json(const json &j, ProcessInfo &p) {
  j.at("pid").get_to(p.pid);
  j.at("vmRssKb").get_to(p.vmRssKb);
  j.at("cpu_percent").get_to(p.cpu_percent);
  j.at("mem_percent").get_to(p.mem_percent);
  j.at("name").get_to(p.name);
  j.at("open_fds").get_to(p.open_fds);
  j.at("io_read_bytes").get_to(p.io_read_bytes);
  j.at("io_write_bytes").get_to(p.io_write_bytes);
}

// System Metrics
void to_json(json &j, const SystemMetrics &s) {
  j = json{
      {"cores", s.cores},
      {"cpu_frequency_ghz", s.cpu_frequency_ghz},
      {"cpu_temp_c", s.cpu_temp_c},
      {"meminfo", s.meminfo},
      {"stability", s.stability},
      {"swapinfo", s.swapinfo},
      {"disks", s.disks},
      {"uptime", s.uptime},
      {"load_avg_1m", s.load_avg_1m},
      {"load_avg_5m", s.load_avg_5m},
      {"load_avg_15m", s.load_avg_15m},
      {"processes_total", s.processes_total},
      {"processes_running", s.processes_running},
      {"sys_name", s.sys_name},
      {"node_name", s.node_name},
      {"kernel_release", s.kernel_release},
      {"machine_type", s.machine_type},
      {"network_interfaces", s.network_interfaces},
      {"top_processes_avg_mem", s.top_processes_avg_mem},
      {"top_processes_avg_cpu", s.top_processes_avg_cpu},
      {"top_processes_real_mem", s.top_processes_real_mem},
      {"top_processes_real_cpu", s.top_processes_real_cpu},
      // Note: polling_tasks is intentionally omitted
  };
  j["disk_io"] = json::array();
  for (const auto &pair : s.disk_io) {
    SPDLOG_TRACE("Serializing {}", pair.second.device_name);
    j["disk_io"].push_back(
        pair.second); // pair.second is the DiskIoStats object
  }
}

void from_json(const json &j, SystemMetrics &s) {
  j.at("cores").get_to(s.cores);
  j.at("cpu_frequency_ghz").get_to(s.cpu_frequency_ghz);
  j.at("cpu_temp_c").get_to(s.cpu_temp_c);
  j.at("meminfo").get_to(s.meminfo);
  j.at("stability").get_to(s.stability);
  j.at("swapinfo").get_to(s.swapinfo);
  j.at("disks").get_to(s.disks);
  j.at("uptime").get_to(s.uptime);
  j.at("load_avg_1m").get_to(s.load_avg_1m);
  j.at("load_avg_5m").get_to(s.load_avg_5m);
  j.at("load_avg_15m").get_to(s.load_avg_15m);
  j.at("processes_total").get_to(s.processes_total);
  j.at("processes_running").get_to(s.processes_running);
  j.at("sys_name").get_to(s.sys_name);
  j.at("node_name").get_to(s.node_name);
  j.at("kernel_release").get_to(s.kernel_release);
  j.at("machine_type").get_to(s.machine_type);
  j.at("network_interfaces").get_to(s.network_interfaces);
  j.at("top_processes_avg_mem").get_to(s.top_processes_avg_mem);
  j.at("top_processes_avg_cpu").get_to(s.top_processes_avg_cpu);
  j.at("top_processes_real_mem").get_to(s.top_processes_real_mem);
  j.at("top_processes_real_cpu").get_to(s.top_processes_real_cpu);
  j.at("disk_io").get_to(s.disk_io);
  // Note: polling_tasks is intentionally omitted
}

void to_json(json &j, const SystemStability &s) {
  j = json{
      {"file_descriptors",
       {{"allocated", s.file_descriptors_allocated},
        {"max", s.file_descriptors_max}}},
      {"pressure",
       {{"memory",
         {{"some", s.memory_pressure_some}, {"full", s.memory_pressure_full}}},
        {"io", {{"some", s.io_pressure_some}, {"full", s.io_pressure_full}}}}},
      {"memory_fragmentation_index", s.memory_fragmentation_index}};
}

void from_json(const json &j, SystemStability &s) {
  // Parsing nested file_descriptors
  if (j.contains("file_descriptors")) {
    const auto &fd = j.at("file_descriptors");
    fd.at("allocated").get_to(s.file_descriptors_allocated);
    fd.at("max").get_to(s.file_descriptors_max);
  }

  // Parsing nested pressure
  if (j.contains("pressure")) {
    const auto &pr = j.at("pressure");

    if (pr.contains("memory")) {
      const auto &mem = pr.at("memory");
      mem.at("some").get_to(s.memory_pressure_some);
      mem.at("full").get_to(s.memory_pressure_full);
    }

    if (pr.contains("io")) {
      const auto &io = pr.at("io");
      io.at("some").get_to(s.io_pressure_some);
      io.at("full").get_to(s.io_pressure_full);
    }
  }

  // Parsing top-level fragmentation index
  j.at("memory_fragmentation_index").get_to(s.memory_fragmentation_index);
}
}; // namespace telemetry
