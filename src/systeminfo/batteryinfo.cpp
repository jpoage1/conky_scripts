// batteryinfo.cpp
#include "batteryinfo.hpp"

#include "data_local.hpp"
#include "data_ssh.hpp"

std::vector<BatteryStatus> LocalDataStreams::get_battery_status(
    const std::vector<BatteryConfig>& configs) {
  std::vector<BatteryStatus> results;

  for (const auto& config : configs) {
    BatteryStatus stat;
    stat.name = config.name;
    stat.path = config.path;

    std::filesystem::path p(config.path);
    if (std::filesystem::exists(p)) {
      // Read Capacity
      std::ifstream cap_file(p / "capacity");
      if (cap_file) cap_file >> stat.percentage;

      // Read Status
      std::ifstream status_file(p / "status");
      if (status_file) {
        std::string s;
        std::getline(status_file, s);
        if (!s.empty()) stat.status = s;
      }
    }
    results.push_back(stat);
  }
  return results;
}
std::vector<BatteryStatus> ProcDataStreams::get_battery_status(
    const std::vector<BatteryConfig>& /*configs*/) {
  // SSH Battery monitoring not yet implemented
  return {};
}
