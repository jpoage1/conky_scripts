// batteryinfo.cpp
#include "batteryinfo.hpp"
#include "context.hpp"
#include "polling.hpp"

#include "data_local.hpp"
#include "data_ssh.hpp"

std::vector<BatteryStatus> LocalDataStreams::get_battery_status(
    const std::vector<BatteryConfig> &configs) {
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
std::vector<BatteryStatus> ProcDataStreams::get_battery_status(
    const std::vector<BatteryConfig> & /*configs*/) {
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
