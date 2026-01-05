#include "lua_parser.hpp"

#include "cli_parser.hpp"
#include "config_types.hpp"
#include "context.hpp"
#include "log.hpp"

sol::state load_lua_file(const std::string& filename) {
  sol::state lua;

  // 1. Open standard libraries
  lua.open_libraries(sol::lib::base, sol::lib::os, sol::lib::string,
                     sol::lib::package, sol::lib::io);

  // 2. Load the file safely
  try {
    lua.script_file(filename);
  } catch (const sol::error& e) {
    std::cerr << "Lua Config Error: " << e.what() << std::endl;
  }

  return lua;
}

ParsedConfig load_lua_config(const std::string& filename) {
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

MetricsContext load_lua_settings(const std::string& filename) {
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
  ParsedConfig config;

  // =========================================================
  // TOP LEVEL SETTINGS
  // =========================================================

  if (lua_config["log_level"].valid()) {
    std::string level_str = lua_config.get<std::string>("log_level");
    configure_log_level(level_str);
  } else {
    std::cerr << "Configuring log level: Skipping!" << std::endl;
    configure_log_level("off");
  }

  // Map string enums
  std::string run_mode_str =
      lua_config.get_or<std::string>("run_mode", "persistent");

  config.set_run_mode(run_mode_str);

  std::string output_fmt =
      lua_config.get_or<std::string>("output_format", "json");
  config.set_output_mode(output_fmt);

  // Polling Interval
  int interval_ms = lua_config.get_or("polling_interval_ms", 2500);
  config.set_polling_interval(std::chrono::milliseconds(interval_ms));

  if (lua_config["settings"].valid()) {
    // Sol2 Magic: Get array of tables
    std::vector<sol::table> settings_list =
        lua_config["settings"].get<std::vector<sol::table>>();

    for (auto& settings : settings_list) {
      MetricsContext context = parse_settings(settings);
      config.tasks.push_back(std::move(context));
    }
  }

  return config;
}

MetricsContext parse_settings(sol::table lua_settings) {
  MetricsContext context;
  MetricSettings& settings = context.settings;

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

    // CORRECT SYNTAX: get<optional>().value_or()
    settings.enable_sysinfo =
        features.get<sol::optional<bool>>("enable_sysinfo").value_or(true);
    settings.enable_uptime =
        features.get<sol::optional<bool>>("enable_uptime").value_or(true);
    settings.enable_memory =
        features.get<sol::optional<bool>>("enable_memory").value_or(true);
    settings.enable_cpu_temp =
        features.get<sol::optional<bool>>("enable_cpu_temp").value_or(true);
    settings.enable_cpuinfo =
        features.get<sol::optional<bool>>("enable_cpuinfo").value_or(true);
    settings.enable_load_and_process_stats =
        features.get<sol::optional<bool>>("enable_load_and_process_stats")
            .value_or(true);
    settings.enable_network_stats =
        features.get<sol::optional<bool>>("enable_network_stats")
            .value_or(true);
    settings.enable_diskstat =
        features.get<sol::optional<bool>>("enable_diskstat").value_or(true);

    if (features["processes"].valid()) {
      sol::table procs = features["processes"];
      settings.enable_avg_processinfo_cpu =
          procs.get<sol::optional<bool>>("enable_avg_cpu").value_or(true);
      settings.enable_avg_processinfo_mem =
          procs.get<sol::optional<bool>>("enable_avg_mem").value_or(true);
      settings.enable_realtime_processinfo_cpu =
          procs.get<sol::optional<bool>>("enable_realtime_cpu").value_or(true);
      settings.enable_realtime_processinfo_mem =
          procs.get<sol::optional<bool>>("enable_realtime_mem").value_or(true);
      settings.only_user_processes =
          procs.get<sol::optional<bool>>("only_user_processes").value_or(true);
      settings.process_count =
          procs.get<sol::optional<long unsigned int>>(std::string("count"))
              .value_or(10);
      settings.ignore_list =
          procs.get<sol::optional<std::vector<std::string>>>("ignore_list")
              .value_or(std::vector<std::string>{});
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
  // BATTERIES (Lists)
  // =========================================================
  if (lua_settings["batteries"].valid()) {
    sol::table batteries_list = lua_settings["batteries"];

    // Iterate over the array part of the table
    // sol::table iteration returns {key, value} pairs
    for (const auto& kv : batteries_list) {
      sol::object value = kv.second;

      // Ensure the item is actually a table (e.g. { name=..., path=... })
      if (value.is<sol::table>()) {
        sol::table bat_item = value.as<sol::table>();

        BatteryConfig config;
        config.name =
            bat_item.get_or<std::string>(std::string("name"), "Battery");
        // config.name =
        // bat_item.get<sol::optional<std::string>>("name").value_or(
        //     "Battery");
        config.path = bat_item.get_or<std::string>("path", "");

        // Basic validation
        if (!config.path.empty()) {
          settings.batteries.push_back(config);
        }
      }
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

    // Filter Config
    if (storage["filters"].valid()) {
      sol::table filters = storage["filters"];

      if (filters.get_or("enable_loopback", false)) {
        context.disk_stat_config.insert(DiskStatSettings::Loopback);
      }
      if (filters.get_or("enable_mapper", false)) {
        context.disk_stat_config.insert(DiskStatSettings::MapperDevices);
      }
      if (filters.get_or("enable_partitions", false)) {
        context.disk_stat_config.insert(DiskStatSettings::Partitions);
      }
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
      ssh.get<sol::optional<int>>(std::string("port"));
      context.key = ssh.get_or<std::string>("key_path", "");
    } else {
      context.provider = DataStreamProviders::LocalDataStream;
    }
  }

  return context;
}
