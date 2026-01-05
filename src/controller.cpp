// controller.cpp
#include "controller.hpp"

#include <list>
#include <memory>

#include "config_types.hpp"
#include "log.hpp"
#include "metrics.hpp"
#include "polling.hpp"

struct Controller::SystemMetricsImpl {
  std::unique_ptr<ParsedConfig> config;
  std::list<SystemMetrics> tasks;  // Matches your signature perfectly
};
Controller::Controller() : tasks_pimpl(std::make_unique<SystemMetricsImpl>()) {}
Controller::~Controller() = default;

void Controller::initialize(ParsedConfig& config) {
  // 1. Move or clone the config into the managed unique_ptr
  // If ParsedConfig has a move constructor:
  tasks_pimpl->config = std::make_unique<ParsedConfig>(std::move(config));

  // 2. Initialize the system tasks
  // Note: This requires the ParsedConfig::initialize signature
  // to accept std::list<std::unique_ptr<SystemMetrics>>&
  tasks_pimpl->config->initialize(tasks_pimpl->tasks);
}

// This is the core execution step logic
void Controller::tick() {
  if (tasks_pimpl->config->reload_if_changed(tasks_pimpl->tasks)) {
    return;
  }

  for (SystemMetrics& task : tasks_pimpl->tasks) {
    DEBUG_PTR("main SystemMetrics task address", task);
    if (task.read_data() != 0) {
      SPDLOG_WARN("Warning: Failed to read initial data for task.");
    }
    SPDLOG_TRACE("Running task");
    for (std::unique_ptr<IPollingTask>& polling_task : task.polling_tasks) {
      DEBUG_PTR("Polling task address", polling_task);
      polling_task->take_new_snapshot();
      polling_task->calculate();
      polling_task->commit();
    }

    // refresh data after polling
    //   task.complete(); // not implemented

    SPDLOG_TRACE("Calling cleanup(): Cleaning up data provider.");
    task.provider->cleanup();
  }
  SPDLOG_TRACE("Calling config.done()");
  tasks_pimpl->config->done(tasks_pimpl->tasks);
}

bool Controller::is_persistent() const {
  return tasks_pimpl->config->run_mode(RunMode::PERSISTENT);
}

void Controller::sleep() { tasks_pimpl->config->sleep(); }

void Controller::inject_task(MetricsContext&& context) {
  tasks_pimpl->config->tasks.push_back(std::move(context));
}
