// batteryinfo.cpp
#include "batteryinfo.hpp"
#include "context.hpp"
#include "polling.hpp"

#include "data_local.hpp"
#include "data_ssh.hpp"

namespace telemetry {

std::vector<BatteryStatus>

LocalDataStreams::get_battery_status(const Batteries &configs) {
  std::vector<BatteryStatus> results;

  for (const auto &config : configs) {
    BatteryStatus stat;
    stat.name = config.name;
    stat.path = config.path;

    std::filesystem::path p(config.path);
    if (std::filesystem::exists(p)) {
      // Read Capacity
      std::ifstream cap_file(p / "capacity");
      if (cap_file)
        cap_file >> stat.percentage;

      // Read Status
      std::ifstream status_file(p / "status");
      if (status_file) {
        std::string s;
        std::getline(status_file, s);
        if (!s.empty())
          stat.status = s;
      }
    }
    results.push_back(stat);
  }
  return results;
}
std::vector<BatteryStatus>
ProcDataStreams::get_battery_status(const Batteries & /*configs*/) {
  // SSH Battery monitoring not yet implemented
  return {};
}

BatteryPollingTask::BatteryPollingTask(DataStreamProvider &p, SystemMetrics &m,
                                       MetricsContext &ctx)
    : IPollingTask(p, m, ctx), configs(ctx.settings.batteries) {
  name = "Battery Polling";
}

void BatteryPollingTask::configure() {}
void BatteryPollingTask::take_initial_snapshot() { take_new_snapshot(); }
void BatteryPollingTask::take_new_snapshot() {
  current_status = provider.get_battery_status(configs);
}
void BatteryPollingTask::calculate() { metrics.battery_info = current_status; }
void BatteryPollingTask::commit() {}

BatteryStatus::BatteryStatus(Value v = UNKNOWN) : m_val(v) {}

operator BatteryStatus::Value() const { return m_val; }

static BatteryStatus::std::string get_str(Value v) {
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

const BatteryStatus::std::string get_str() const {
  return BatteryStatus::get_str(m_val);
}

void BatteryStatus::set(const std::string &val) {
  if (val == "Charging")
    m_val = CHARGING;
  else if (val == "Discharging")
    m_val = DISCHARGING;
  else if (val == "Full")
    m_val = FULL;
  else
    m_val = UNKNOWN;
}

Battery::Battery(std::string p) : path(p) {}
Battery::Battery(std::string n, std::string p) : name(n), path(p) {}

Batteries::Batteries() {
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

iterator Batteries::begin() { return m_list.begin(); }
iterator Batteries::end() { return m_list.end(); }
const_iterator Batteries::begin() const { return m_list.begin(); }
const_iterator Batteries::end() const { return m_list.end(); }
size_t Batteries::size() const { return m_list.size(); }
bool Batteries::empty() const { return m_list.empty(); }
const Battery Batteries::&operator[](size_t i) const { return m_list[i]; }

std::string LuaBatteryStatus ::serialize(unsigned indentation_level = 0) const {
  LuaConfigLuaConfigGenerator gen(indentation_level);
  gen.lua_string("status", get_str());
  return gen.str();
} // end BatteryStatus::serialize()

std::string LuaBattery::serialize(unsigned indentation_level = 0) const {
  LuaConfigLuaConfigGenerator gen(indentation_level);
  gen.lua_string("name", name);
  gen.lua_string("path", path);
  gen.lua_int("critical_threshold", critical_threshold);
  gen.lua_string("icon", icon);
  gen.lua_string("status", status.get_str());

  return gen.str();
} // End Battery::serialize

void LuaBattery::deserialize(const sol::table &bat_item) {
  name = bat_item.get_or("name", std::string("Battery"));
  path = bat_item.get_or("path", std::string(""));
  critical_threshold = bat_item.get_or("critical_threshold", 15);
  icon = bat_item.get_or("icon", std::string("⚡"));

  if (bat_item["status"].valid()) {
    status.set(bat_item.get<std::string>("status"));
  }
}

std::string LuaBatteries::serialize(unsigned indentation_level = 0) const {
  LuaConfigLuaConfigGenerator gen("batteries", indentation_level);
  for (const Battery &batt : m_list) {
    gen.lua_list_raw_entry(batt.serialize());
  }
  return gen.str();
} // End Batteries::serialize()

void LuaBatteries::deserialize(sol::table batteries_table) {
  if (!batteries_table.valid())
    return;

  // 1. Clear existing list if this is a reload
  this->m_list.clear();

  // 2. Iterate over the array part of the table
  for (const auto &kv : batteries_table) {
    sol::object value = kv.second;

    if (value.is<sol::table>()) {
      // 3. Use the Lua wrapper to handle the individual battery logic
      // This uses the LuaBattery::deserialize you already implemented
      LuaBattery lb("");
      lb.deserialize(value.as<sol::table>());

      // 4. Validate and store the base POD struct
      if (!lb.path.empty()) {
        // We cast to the base 'Battery' struct to strip away the wrapper
        // logic
        this->m_list.push_back(static_cast<Battery>(lb));
      }
    }
  }
}

}; // namespace telemetry
