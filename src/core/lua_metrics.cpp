#include "lua_metrics.hpp"
#include "batteryinfo.hpp"
#include "diskstat.hpp"
#include "lua_generator.hpp"
#include "metric_settings.hpp"
#include "networkstats.hpp"
#include "processinfo.hpp"
#include "types.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sol/sol.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace telemetry {
struct LuaConfigGenerator;
using Generator = LuaConfigGenerator;

std::string LuaFeatures::serialize(unsigned const int indentation_level) const {
  Generator features("features", indentation_level);

  // Core Metrics
  features.lua_bool("enable_sysinfo", enable_sysinfo);
  features.lua_bool("enable_uptime", enable_uptime);
  features.lua_bool("enable_memory", enable_memory);
  features.lua_bool("enable_cpu_temp", enable_cpu_temp);
  features.lua_bool("enable_cpuinfo", enable_cpuinfo);

  // Stats and Logic
  features.lua_bool("enable_load_and_process_stats",
                    enable_load_and_process_stats);
  features.lua_bool("enable_network_stats", enable_network_stats);
  features.lua_bool("enable_diskstat", enable_diskstat);
  features.lua_bool("enable_network_stats", enable_network_stats);

  return features.str();
} // End Features::serialize()

void LuaFeatures::deserialize(sol::table features) {
  if (features.valid()) {
    enable_sysinfo =
        features.get<sol::optional<bool>>("enable_sysinfo").value_or(true);
    enable_uptime =
        features.get<sol::optional<bool>>("enable_uptime").value_or(true);
    enable_memory =
        features.get<sol::optional<bool>>("enable_memory").value_or(true);
    enable_cpu_temp =
        features.get<sol::optional<bool>>("enable_cpu_temp").value_or(true);
    enable_cpuinfo =
        features.get<sol::optional<bool>>("enable_cpuinfo").value_or(true);
    enable_load_and_process_stats =
        features.get<sol::optional<bool>>("enable_load_and_process_stats")
            .value_or(true);
    enable_network_stats =
        features.get<sol::optional<bool>>("enable_network_stats")
            .value_or(true);
    enable_diskstat =
        features.get<sol::optional<bool>>("enable_diskstat").value_or(true);
    if (features["processes"].valid()) {
      LuaProcesses lp;
      lp.deserialize(features["processes"]);
      // Slice/Assign back to base struct
      processes = static_cast<Processes>(lp);
    }
  }
}

std::string LuaMetricSettings::serialize(unsigned indentation_level) const {
  // Empty name results in Generator::str() calling lua_wrap()
  // producing an anonymous table { ... }
  Generator gen("", indentation_level);

  // 1. Serialize top-level primitives
  gen.lua_string("name", name);

  // 2. Append serialized blocks from inner structs
  // Each of these handles its own 'key = { ... }' wrapping
  gen.lua_append(
      static_cast<const LuaFeatures &>(features).serialize(indentation_level));
  gen.lua_append(static_cast<const LuaBatteries &>(batteries).serialize(
      indentation_level));
  gen.lua_append(
      static_cast<const LuaStorage &>(storage).serialize(indentation_level));
  gen.lua_append(static_cast<const LuaSSH &>(ssh).serialize(indentation_level));
  gen.lua_append(static_cast<const LuaWindowConfig &>(window).serialize(
      indentation_level));

  return gen.str();
} // End MetricSettings::serialize

void LuaMetricSettings::deserialize(sol::table settings) {
  if (!settings.valid())
    return;

  name = settings.get_or("name", std::string("unnamed_source"));

  if (settings["features"].valid()) {
    LuaFeatures lf;
    lf.deserialize(settings["features"]);
    features = static_cast<Features>(lf);
  }

  if (settings["batteries"].valid()) {
    LuaBatteries lb;
    lb.deserialize(settings["batteries"]);
    batteries = static_cast<Batteries>(lb);
  }

  if (settings["storage"].valid()) {
    LuaStorage ls;
    ls.deserialize(settings["storage"]);
    storage = static_cast<Storage>(ls);
  }

  if (settings["ssh"].valid()) {
    LuaSSH ls;
    ls.deserialize(settings["ssh"]);
    ssh = static_cast<SSH>(ls);
  }

  if (settings["window"].valid()) {
    LuaWindowConfig lw;
    lw.deserialize(settings["window"]);
    window = static_cast<WindowConfig>(lw);
  }
}

std::string LuaMetricsConfig::serialize() const {
  // Top-level config usually starts at indentation 0
  Generator gen("config", 0);

  // 1. Global Configuration Primitives
  gen.lua_string("run_mode", run_mode);
  gen.lua_string("output_format", output_format);
  gen.lua_int("polling_interval_ms", polling_interval_ms);
  gen.lua_string("log_level", log_level);
  gen.lua_bool("dump_to_file", dump_to_file);
  gen.lua_string("log_file_path", log_file_path);

  // 2. Aggregate serialized MetricSettings
  std::vector<std::string> serialized_settings;
  for (const MetricSettings &s : settings) {
    const auto &lua_s = static_cast<const LuaMetricSettings &>(s);
    // Increment indentation for the settings list content
    serialized_settings.push_back(lua_s.serialize(1));
  }

  // 3. Serialize the settings list as raw Lua tables
  gen.lua_raw_list("settings", serialized_settings);

  // 4. Finalize with Lua return statement
  return gen.str() + "\nreturn config";
} // End MetricsConfig::serialize

void LuaMetricsConfig::deserialize(sol::table config) {
  if (!config.valid())
    return;

  run_mode = config.get_or("run_mode", std::string("persistent"));
  output_format = config.get_or("output_format", std::string("json"));
  polling_interval_ms = config.get_or("polling_interval_ms", 1000);
  log_level = config.get_or("log_level", std::string("warn"));
  dump_to_file = config.get_or("dump_to_file", false);
  log_file_path =
      config.get_or("log_file_path", std::string("/tmp/telemetry.log"));

  if (config["settings"].valid()) {
    sol::table settings_list = config["settings"];
    settings.clear();
    for (auto &kv : settings_list) {
      if (kv.second.is<sol::table>()) {
        LuaMetricSettings lms;
        lms.deserialize(kv.second.as<sol::table>());
        settings.push_back(static_cast<MetricSettings>(lms));
      }
    }
  }
}

}; // namespace telemetry
