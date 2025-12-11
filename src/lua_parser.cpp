#include "lua_parser.hpp"

#include <sol/sol.hpp>

#include "parser.hpp"

MetricsContext load_lua_config(const std::string& filename) {
  // ParsedConfig config;
  MetricsContext context;
  MetricSettings& settings = context.settings;
  sol::state lua;

  // 1. Open standard libraries
  lua.open_libraries(sol::lib::base, sol::lib::os, sol::lib::string,
                     sol::lib::package, sol::lib::io);

  // 2. Load the file safely
  try {
    lua.script_file(filename);
  } catch (const sol::error& e) {
    std::cerr << "Lua Config Error: " << e.what() << std::endl;
    exit(1);
  }

  // 3. Access 'settings' table
  if (!lua["settings"].valid()) {
    std::cerr << "Error: 'settings' table missing in " << filename << std::endl;
    return context;
  }

  sol::table lua_settings = lua["settings"];

  // =========================================================
  // TOP LEVEL SETTINGS
  // =========================================================
  // Map string enums
  // std::string run_mode_str =
  //     lua_settings.get_or<std::string>("run_mode", "persistent");

  // config.set_run_mode(run_mode_str);

  // std::string output_fmt =
  //     lua_settings.get_or<std::string>("output_format", "json");
  // config.set_output_mode(output_fmt);

  // Polling Interval
  // config.set_polling_interval = lua_settings.get_or("polling_interval_ms",
  // 2500);

  // System Name (Calculated in Lua)
  if (lua_settings["name"].valid()) {
    // Assuming you have a name field in MetricSettings, or use it for context
    context.source_name = lua_settings.get<std::string>("name");
  }

  // =========================================================
  // FEATURE TOGGLES (Nested in 'features')
  // =========================================================
  if (lua_settings["features"].valid()) {
    sol::table features = lua_settings["features"];

    settings.enable_sysinfo = features.get_or("enable_sysinfo", true);
    settings.enable_uptime = features.get_or("enable_uptime", true);
    settings.enable_memory = features.get_or("enable_memory", true);
    settings.enable_cpu_temp = features.get_or("enable_cpu_temp", true);
    settings.enable_cpuinfo = features.get_or("enable_cpuinfo", true);
    settings.enable_load_and_process_stats =
        features.get_or("enable_load_and_process_stats", true);
    settings.enable_network_stats =
        features.get_or("enable_network_stats", true);
    settings.enable_diskstat = features.get_or("enable_diskstat", true);

    // Processes (Double Nested)
    if (features["processes"].valid()) {
      sol::table procs = features["processes"];
      settings.enable_avg_processinfo_cpu =
          procs.get_or("enable_avg_cpu", true);
      settings.enable_avg_processinfo_mem =
          procs.get_or("enable_avg_mem", true);
      settings.enable_realtime_processinfo_cpu =
          procs.get_or("enable_realtime_cpu", true);
      settings.enable_realtime_processinfo_mem =
          procs.get_or("enable_realtime_mem", true);

      // Not implemented
      // settings.process_count = procs.get_or("count", 10);
    }
  }

  // =========================================================
  // NETWORK (Lists)
  // =========================================================
  if (lua_settings["network"].valid()) {
    sol::table net = lua_settings["network"];

    // Extract Interface List
    if (net["interfaces"].valid()) {
      // Auto-convert Lua array to C++ vector
      context.interfaces = net["interfaces"].get<std::set<std::string>>();
    }
  }

  // =========================================================
  // STORAGE (Lists)
  // =========================================================
  if (lua_settings["storage"].valid()) {
    sol::table storage = lua_settings["storage"];

    if (storage["filesystems"].valid()) {
      context.filesystems =
          storage["filesystems"].get<std::vector<std::string>>();
    }

    if (storage["io_devices"].valid()) {
      context.io_devices =
          storage["io_devices"].get<std::vector<std::string>>();
    }
  }

  // =========================================================
  // DATA PROVIDER CONFIGURATION
  // =========================================================
  if (lua_settings["ssh"].valid()) {
    sol::table ssh = lua_settings["ssh"];

    // This maps to the logic you previously had in process_command
    bool ssh_enabled = ssh.get_or("enabled", false);

    if (ssh_enabled) {
      context.provider = DataStreamProviders::ProcDataStream;

      context.host = ssh.get_or<std::string>("host", "");
      context.user = ssh.get_or<std::string>("user", "");
      context.port = ssh.get_or("port", 22);
      context.key = ssh.get_or<std::string>("key_path", "");
    } else {
      context.provider = DataStreamProviders::LocalDataStream;
    }
  }

  return context;
}
