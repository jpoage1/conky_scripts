#include "lua_parser.hpp"

#include "cli_parser.hpp"
#include "context.hpp"
#include "log.hpp"
#include "lua_metrics.hpp"
// #include "metric_settings.hpp"
#include "parsed_config.hpp"

namespace telemetry {

sol::state load_lua_file(const std::string &filename) {
  sol::state lua;

  // 1. Open standard libraries
  lua.open_libraries(sol::lib::base, sol::lib::os, sol::lib::string,
                     sol::lib::package, sol::lib::io);

  // 2. Load the file safely
  try {
    lua.script_file(filename);
  } catch (const sol::error &e) {
    std::cerr << "Lua Config Error: " << e.what() << std::endl;
  }

  return lua;
}

ParsedConfig load_lua_config(const std::string &filename) {
  ParsedConfig config;
  sol::state lua = load_lua_file(filename);

  if (!lua["config"].valid()) {
    std::cerr << "Error: 'config' table missing in " << filename << std::endl;
    return config;
  }

  sol::table lua_config = lua["config"];

  config = parse_config(lua_config);

  return config;
}

MetricsContext load_lua_settings(const std::string &filename) {
  MetricsContext context;
  sol::state lua = load_lua_file(filename);

  if (!lua["settings"].valid()) {
    std::cerr << "Error: 'settings' table missing in " << filename << std::endl;
    return context;
  }

  sol::table lua_settings = lua["settings"];

  context = parse_settings(lua_settings);

  return context;
}

ParsedConfig parse_config(sol::table lua_config) {
  // 1. Use the wrapper to handle the heavy lifting
  LuaMetricsConfig lmc;
  lmc.deserialize(lua_config);

  // 2. Map global configuration to ParsedConfig
  ParsedConfig config;
  config.set_run_mode(lmc.run_mode);
  config.set_output_mode(lmc.output_format);
  config.set_polling_interval(
      std::chrono::milliseconds(lmc.polling_interval_ms));

  // Global side-effect: log level
  configure_log_level(lmc.log_level);

  // 3. Convert serialized settings into MetricsContext tasks
  if (lua_config["settings"].valid()) {
    sol::table settings_list = lua_config["settings"];
    for (auto &kv : settings_list) {
      if (kv.second.is<sol::table>()) {
        config.tasks.push_back(parse_settings(kv.second.as<sol::table>()));
      }
    }
  }

  return config;
}
MetricsContext parse_settings(sol::table lua_settings) {
  // 1. Leverage the wrapper for recursive deserialization
  LuaMetricSettings lms;
  lms.deserialize(lua_settings);

  // 2. Map data to the application's context
  MetricsContext context;
  context.settings = static_cast<MetricSettings>(lms); // Slice to base POD
  context.source_name = lms.name;

  // 3. Map complex state (SSH/Local Provider)
  // The wrapper already populated lms.ssh during deserialize()
  if (lms.ssh.enabled) {
    context.provider = DataStreamProviders::ProcDataStream;
    context.host = lms.ssh.host;
    context.user = lms.ssh.user;
    context.key = lms.ssh.key_path;
  } else {
    context.provider = DataStreamProviders::LocalDataStream;
  }

  // 4. Map inherited lists (Network/Storage)
  // Since lms inherits from MetricSettings, these are already populated
  context.interfaces = context.settings.interfaces;
  context.filesystems = context.settings.filesystems;

  return context;
}
}; // namespace telemetry
