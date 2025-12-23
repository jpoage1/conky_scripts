// json_definitions.cpp
#include "json_definitions.hpp"

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
void to_json(json& j, const ProcessInfo& p) {
  j = json{{"pid", p.pid},
           {"vmRssKb", p.vmRssKb},
           {"cpu_percent", p.cpu_percent},
           {"mem_percent", p.mem_percent},
           {"name", p.name}};
}

void from_json(const json& j, ProcessInfo& p) {
  j.at("pid").get_to(p.pid);
  j.at("vmRssKb").get_to(p.vmRssKb);
  j.at("cpu_percent").get_to(p.cpu_percent);
  j.at("mem_percent").get_to(p.mem_percent);
  j.at("name").get_to(p.name);
}

void to_json(json& j, const Time& t) {
  j = json{
      {"days", t.days},
      {"hours", t.hours},
      {"minutes", t.minutes},
      {"seconds", t.seconds},
      {"raw", t.raw},
      {"text", t.to_str()},        // "0d 0h 5m"
      {"clock", t.to_clock_str()}  // "00:00:05"
  };
}

void from_json(const json& j, Time& t) {
  j.at("days").get_to(t.days);
  j.at("hours").get_to(t.hours);
  j.at("minutes").get_to(t.minutes);
  j.at("seconds").get_to(t.seconds);
  j.at("raw").get_to(t.raw);
}

// 4. Define serialization for the PARENT STRUCT (SystemMetrics) LAST
//    Now it can find the definitions for all its members.
void to_json(json& j, const SystemMetrics& s) {
  j = json{
      {"cores", s.cores},
      {"cpu_frequency_ghz", s.cpu_frequency_ghz},
      {"cpu_temp_c", s.cpu_temp_c},
      {"meminfo", s.meminfo},
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
  for (const auto& pair : s.disk_io) {
    SPDLOG_TRACE("Serializing {}", pair.second.device_name);
    j["disk_io"].push_back(
        pair.second);  // pair.second is the DiskIoStats object
  }
}

void from_json(const json& j, SystemMetrics& s) {
  j.at("cores").get_to(s.cores);
  j.at("cpu_frequency_ghz").get_to(s.cpu_frequency_ghz);
  j.at("cpu_temp_c").get_to(s.cpu_temp_c);
  j.at("meminfo").get_to(s.meminfo);
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
