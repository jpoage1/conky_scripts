#ifndef METRIC_SETTINGS_HPP
#define METRIC_SETTINGS_HPP

#include "lua_generator.hpp"
#include "metric_settings.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using fs = std::filesystem;
namespace telemetry {

using Generator = LuaConfigGenerator;
struct MetricsConfig {
  struct MetricSettings {

    // Window Config
    struct WindowConfig {
      enum WindowTypes { DESKTOP, DOCK, NORMAL };
      enum StackingTypes { FOREGROUND, BACKGROUND, FLOATING };
      struct ScrollDirections {
        class ScrollState {
          enum Value { ON, OFF, AUTO };

        public:
          ScrollState(Value v = OFF) : m_val(v) {}
          operator Value() const { return m_val; }
          bool on() const { return m_val != OFF; }
          bool off() const { return m_val != ON; }
          bool is_auto() const { return m_val == AUTO; }
          static std::string get_str(Value &v) {
            switch (v) {
            case ON:
              return "on";
            case OFF:
              return "off";
            case AUTO:
              return "auto";
            default:
              return "unknown";
            }
          } // End get_str()
          const std::string get_str() { return ScrollState::get_str(m_val); }
          void set(std::string val) {
            if (val == "on") {
              m_val = ON;
            } else if (val == "off") {
              m_val = OFF;
            } else if (val == "auto") {
              m_val = AUTO;
            } else {
              m_val = OFF; // Fallback for unknown input
            }
          } // End set()

          std::string serialize(std::string name,
                                unsigned int indentation_level = 0) {
            Generator gen(indentation_level);
            gen.lua_string(name, get_str());
            return gen.raw_str();
          } // End ScrollState::serialize(), redundant and useless

        private:
          Value m_val;
        }; // End ScrollState class

      public:
        ScrollState horizontal;
        ScrollState vertical;

        std::string serialize(unsigned int indentation_level = 0) const {
          Generator gen("scroll", indentation_level);
          gen.lua_string("horizontal", horizontal.get_str());
          gen.lua_string("vertical", vertical.get_str());
          return gen.str();
        } // End ScrollDirections::serialize()
      }; // End ScrollDirections

      ScrollDirections scroll;
      std::string type = "normal";
      std::string stacking = "bottom";
      bool wmIgnore = true;
      int x = 0, y = 0, width = 800, height = 600;
      bool visible = true;
      bool resizable = true;
      std::string serialize(unsigned int indentation_level = 0) const {
        Generator gen("window", indentation_level);

        // Primitive types
        gen.lua_string("type", type);
        gen.lua_string("stacking", stacking);
        gen.lua_bool("wmIgnore", wmIgnore);
        gen.lua_int("x", x);
        gen.lua_int("y", y);
        gen.lua_int("width", width);
        gen.lua_int("height", height);
        gen.lua_bool("visible", visible);
        gen.lua_bool("resizable", resizable);

        // Nested ScrollDirections serialization
        // This assumes your Generator::str() or a raw append method
        // integrates the sub-table string directly into the stream.
        return gen.str();
      } // End WindowConfig::serialize()
    }; // End WindowConfig struct

    // Features
    struct Features {
      struct Processes {
        bool enable_avg_cpu = true;
        bool enable_avg_mem = true;
        bool enable_realtime_cpu = true;
        bool enable_realtime_mem = true;
        long unsigned int count = true;
        std::vector<std::string> ignore_list;
        bool only_user_processes = false;
        bool enable_processinfo() {
          return enable_avg_cpu || enable_avg_mem || enable_realtime_cpu ||
                 enable_realtime_mem;
        }
        std::string serialize(unsigned const int indentation_level = 0) const {
          Generator processes("processes", indentation_level);
          processes.lua_bool("enable_processinfo", enable_processinfo());
          processes.lua_bool("enable_avg_cpu", enable_avg_cpu);
          processes.lua_bool("enable_avg_mem", enable_avg_mem);
          processes.lua_bool("enable_realtime_cpu", enable_realtime_cpu);
          processes.lua_bool("enable_realtime_mem", enable_realtime_mem);
          processes.lua_uint("count", count);
          processes.lua_vector("ignore_list", ignore_list); // fixme
          return processes.str();
        }
      }; // End Processes struct
      bool enable_sysinfo = true;
      bool enable_memory = true;
      bool enable_cpuinfo = true;
      bool enable_cpu_temp = true;
      bool enable_uptime = true;
      bool enable_load_and_process_stats = true;
      bool enable_diskstat = true;
      bool enable_network_stats = true;
      Processes processes;

      std::string serialize(unsigned const int indentation_level = 0) const {
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
    }; // End Features struct

    // Batteries
    struct Batteries {
      struct Battery {
        class BatteryStatus {
        public:
          enum Value { CHARGING, DISCHARGING, FULL, UNKNOWN };

          BatteryStatus(Value v = UNKNOWN) : m_val(v) {}

          operator Value() const { return m_val; }

          static std::string get_str(Value v) {
            switch (v) {
            case CHARGING:
              return "Charging";
            case DISCHARGING:
              return "Discharging";
            case FULL:
              return "Full";
            default:
              return "Unknown";
            }
          }

          const std::string get_str() const {
            return BatteryStatus::get_str(m_val);
          }

          void set(const std::string &val) {
            if (val == "Charging")
              m_val = CHARGING;
            else if (val == "Discharging")
              m_val = DISCHARGING;
            else if (val == "Full")
              m_val = FULL;
            else
              m_val = UNKNOWN;
          }
          std::string serialize(unsigned indentation_level = 0) const {
            Generator gen(indentation_level);
            gen.lua_string("status", get_str());
            return gen.str();
          } // end BatteryStatus::serialize()

        private:
          Value m_val;
        }; // End BatteryStatus

        std::string name = "Battery";
        std::string path = "/sys/class/power_supply/BAT0";
        int critical_threshold = 15;
        std::string icon = "⚡";
        BatteryStatus status = BatteryStatus::UNKNOWN;

        Battery(std::string p) : path(p) {}
        Battery(std::string n, std::string p) : name(n), path(p) {}
        ~Battery() = default;

        std::string serialize(unsigned indentation_level = 0) const {
          // Generator name is empty because these are array elements {}
          Generator gen("", indentation_level);
          gen.lua_string("name", name);
          gen.lua_string("path", path);
          gen.lua_int("critical_threshold", critical_threshold);
          gen.lua_string("icon", icon);
          // Direct string append for the status field
          gen.lua_mklist("status", status.serialize());

          return gen.str();
        } // End Battery::serialize
      }; // end Battery

      Batteries() {
        const std::string power_supply_dir = "/sys/class/power_supply";
        if (!fs::exists(power_supply_dir))
          return;

        for (const auto &entry : fs::directory_iterator(power_supply_dir)) {
          std::ifstream type_file(entry.path() / "type");
          std::string type;
          if (type_file >> type && type == "Battery") {
            std::string b_path = entry.path().string();
            std::string b_name = entry.path().filename().string();
            m_list.emplace_back(b_name, b_path);
          }
        }
      }
      ~Batteries() = default;

      using iterator = std::vector<Battery>::iterator;
      using const_iterator = std::vector<Battery>::const_iterator;

      iterator begin() { return m_list.begin(); }
      iterator end() { return m_list.end(); }
      const_iterator begin() const { return m_list.begin(); }
      const_iterator end() const { return m_list.end(); }
      size_t size() const { return m_list.size(); }
      bool empty() const { return m_list.empty(); }
      const Battery &operator[](size_t i) const { return m_list[i]; }
      std::string serialize(unsigned indentation_level = 0) const {
        // Empty name produces an anonymous table string: "{ ... }"
        Generator gen("batteries", indentation_level);
        for (Battery batt : m_list) {
          gen.lua_list_raw_entry, batt.serialize());
        }
        return gen.str();
      } // End Batteries::serialize()

    private:
      std::vector<Battery> m_list;
    }; // End Batteries
       //
    struct Network {
      std::vector<std::string> interfaces; // fixme, need a default
      std::string ping_target = "8.8.8.8";
      bool enable_ping = false;

      Network() {
        const std::string net_dir = "/sys/class/net";
        if (!fs::exists(net_dir))
          return;

        for (const auto &entry : fs::directory_iterator(net_dir)) {
          std::string iface = entry.path().filename().string();

          // 1. Skip loopback
          if (iface == "lo")
            continue;

          // 2. Filter for physical devices
          // On Linux, virtual interfaces (lo, veth, br0) lack a 'device'
          // symlink
          if (fs::exists(entry.path() / "device")) {
            interfaces.push_back(iface);
          }
        }

        // Optional: Sort alphabetically for consistent Lua output
        std::sort(interfaces.begin(), interfaces.end());
      }
      ~Network() = default;
      bool has_interfaces() const { return !interfaces.empty(); }
      std::string serialize(unsigned indentation_level = 0) const {
        Generator gen("network", indentation_level);

        // Serialize interfaces as a Lua table array
        gen.lua_vector("interfaces", interfaces);
        gen.lua_string("ping_target", ping_target);
        gen.lua_bool("enable_ping", enable_ping);

        return gen.str();
      } // End Network::serialize()
    }; // end Network struct

    struct Storage {
      struct Filters {
        bool enable_loopback = true;
        bool enable_mapper = true;
        bool enable_partitions = false;
      };
      std::vector<std::string> filesystems;
      std::vector<std::string> io_devices;
      Filters filters;
      Storage() {
        discover_filesystems();
        discover_io_devices();
      }
      std::string serialize(unsigned indentation_level = 0) const {
        Generator gen("storage", indentation_level);

        gen.lua_vector("filesystems", filesystems);
        gen.lua_vector("io_devices", io_devices);

        // Manual insertion of nested filter string to maintain indentation
        // (Assuming Generator::lua_string/lua_raw can be used)
        // For this pattern, we append the serialized filter result:
        // gen.append_raw(filters.serialize(indentation_level + 1));

        return gen.str();
      } // End Storage::serialize()

    private:
      void discover_filesystems() {
        std::ifstream mounts("/proc/mounts");
        std::string line;
        while (std::getline(mounts, line)) {
          std::istringstream iss(line);
          std::string device, mount_point, type;
          if (!(iss >> device >> mount_point >> type))
            continue;

          if (device.find("/dev/") == 0) {
            if (!filters.enable_loopback && device.find("/dev/loop") == 0)
              continue;
            if (!filters.enable_mapper && device.find("/dev/mapper") == 0)
              continue;

            filesystems.push_back(mount_point);
          }
        }
      }

      void discover_io_devices() {
        const std::string block_dir = "/sys/block";
        if (!fs::exists(block_dir))
          return;

        for (const auto &entry : fs::directory_iterator(block_dir)) {
          std::string dev_name = entry.path().filename().string();

          // Filter virtual devices
          if (!filters.enable_loopback && dev_name.find("loop") == 0)
            continue;

          // Physical disks contain a 'device' symlink in sysfs
          // Partitions (sda1) do not; they are subdirectories of the disk (sda)
          if (fs::exists(entry.path() / "device")) {
            io_devices.push_back(dev_name);
          }
        }
      }
    }; // end Storage struct

    struct SSH {
      bool enabled = false;
      std::string host;
      std::string user; // fixme, need default
      std::string key_path = "~/.ssh/id_rsa";
      int timeout_sec = 5;
      bool keepalive = true;

      std::string serialize(unsigned indentation_level = 0) const {
        Generator gen("ssh", indentation_level);

        gen.lua_bool("enabled", enabled);
        gen.lua_string("host", host);
        gen.lua_string("user", user);
        gen.lua_string("key_path", key_path);
        gen.lua_int("timeout_sec", timeout_sec);
        gen.lua_bool("keepalive", keepalive);

        return gen.str();
      } // End SSH::serialize
    }; // End SSH struct

    std::string name;
    Features features;
    Batteries batteries;
    Storage storage;
    SSH ssh;

    WindowConfig window;

    /* LEGACY LOGIC */
    // Core Features
    bool enable_uptime = true;
    bool enable_sysinfo = true;
    bool enable_memory = true;

    // Stats
    bool enable_load_and_process_stats = true;
    bool enable_cpu_temp = true;

    // Stability
    bool enable_stability_info = true;

    bool enable_cpuinfo = true;
    bool enable_network_stats = true;
    bool enable_diskstat = true;
    //
    // ProcessInfo
    bool enable_avg_processinfo_cpu = true;
    bool enable_avg_processinfo_mem = true;
    bool enable_realtime_processinfo_cpu = true;
    bool enable_realtime_processinfo_mem = true;
    long unsigned int process_count = true;
    bool only_user_processes = false;

    // Batteries
    bool enable_battery_info = true;

    std::vector<std::string> ignore_list;
    std::vector<std::string> interfaces;
    std::vector<std::string> filesystems;

    bool enable_processinfo() const {
      return enable_avg_processinfo_cpu || enable_avg_processinfo_mem ||
             enable_realtime_processinfo_cpu || enable_realtime_processinfo_mem;
    }
    std::string serialize(unsigned indentation_level = 0) const {
      // Empty name results in Generator::str() calling lua_wrap()
      // producing an anonymous table { ... }
      Generator gen("", indentation_level);

      // 1. Serialize top-level primitives
      gen.lua_string("name", name);

      // 2. Append serialized blocks from inner structs
      // Each of these handles its own 'key = { ... }' wrapping
      gen.lua_append(features.serialize(indentation_level));
      gen.lua_append(batteries.serialize(indentation_level));
      gen.lua_append(storage.serialize(indentation_level));
      gen.lua_append(ssh.serialize(indentation_level));
      gen.lua_append(window.serialize(indentation_level));

      return gen.str();
    } // End MetricSettings::serialize
  }; // End MetricSettings struct

  std::string run_mode = "persistent";
  std::string output_format = "json";
  int polling_interval_ms = 1000;
  std::string log_level = "warn";
  bool dump_to_file = false;
  std::string log_file_path = "/tmp/telemetery.log";
  std::vector<MetricSettings> settings;
  // Inside struct MetricsConfig
  std::string serialize() const {
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
    for (const auto &s : settings) {
      // Increment indentation for the settings list content
      serialized_settings.push_back(s.serialize(1));
    }

    // 3. Serialize the settings list as raw Lua tables
    gen.lua_raw_list("settings", serialized_settings);

    // 4. Finalize with Lua return statement
    return gen.str() + "\nreturn config";
  } // End MetricConfig::serialize
}; // End MetricConfig struct
}; // namespace telemetry
#endif
