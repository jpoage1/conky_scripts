// batteryinfo.cpp
#include "batteryinfo.hpp"
#include "context.hpp"
#include "lua_generator.hpp"
#include "polling.hpp"

#include "data_local.hpp"
#include "data_ssh.hpp"
#include <string>

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

BatteryStatus::BatteryStatus(BatteryStatus::Value v) : m_val(v) {}

std::string BatteryStatus::get_str(BatteryStatus::Value v) {
  switch (v) {
  case BatteryStatus::CHARGING:
    return "Charging";
  case BatteryStatus::DISCHARGING:
    return "Discharging";
  case BatteryStatus::FULL:
    return "Full";
  default:
    return "Unknown";
  }
}

const std::string BatteryStatus::get_str() const {
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

size_t Batteries::size() const { return m_list.size(); }
bool Batteries::empty() const { return m_list.empty(); }

std::string LuaBatteryStatus::serialize(unsigned int indentation_level) const {
  LuaConfigGenerator gen(indentation_level);
  gen.lua_string("status", get_str());
  return gen.str();
} // end BatteryStatus::serialize()

/* static */
std::string LuaBattery::serialize(const Battery &self,
                                  unsigned int indentation_level) {
  LuaConfigGenerator gen(indentation_level);
  gen.lua_string("name", self.name);
  gen.lua_string("path", self.path);
  gen.lua_int("critical_threshold", self.critical_threshold);
  gen.lua_string("icon", self.icon);
  gen.lua_string("status", self.status.get_str());

  return gen.str();
} // End Battery::serialize

std::string LuaBattery::serialize(unsigned indent) const {
  return LuaBattery::serialize(*this, indent);
}

void LuaBattery::deserialize(Battery &self, const sol::table &bat_item) {
  self.name = bat_item.get_or("name", std::string("Battery"));
  self.path = bat_item.get_or("path", std::string(""));
  self.critical_threshold = bat_item.get_or("critical_threshold", 15);
  self.icon = bat_item.get_or("icon", std::string("⚡"));

  if (bat_item["status"].valid()) {
    self.status.set(bat_item.get<std::string>("status"));
  }
}

void LuaBatteries::deserialize(sol::table batteries_table) {
  if (!batteries_table.valid())
    return;

  this->m_list.clear();

  for (auto &kv : batteries_table) {
    if (kv.second.is<sol::table>()) {
      sol::table item = kv.second.as<sol::table>();
      std::string path = item.get_or("path", std::string(""));

      if (!path.empty()) {
        // 1. Construct the base object with the required path
        Battery b(path);
        // 2. Use your new static method to fill the rest
        LuaBattery::deserialize(b, item);
        // 3. Push the valid POD into the list
        this->m_list.push_back(b);
      }
    }
  }
}
std::string LuaBatteries::serialize(unsigned int indent) const {
  LuaConfigGenerator gen("batteries", indent);
  for (const auto &batt : m_list) {
    // Call the static method directly on the base Battery object
    gen.lua_list_raw_entry(LuaBattery::serialize(batt));
  }
  return gen.str();
}

}; // namespace telemetry
