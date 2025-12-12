// parsed_config.cpp
#include "cli_parser.hpp"
#include "conky_output.hpp"
#include "context.hpp"
#include "log.hpp"
#include "polling.hpp"

bool ParsedConfig::run_mode(RunMode mode) const { return _run_mode == mode; }
bool ParsedConfig::output_mode(OutputMode mode) const {
  return _output_mode == mode;
}
RunMode ParsedConfig::run_mode() const { return _run_mode; }
OutputMode ParsedConfig::get_output_mode() const { return _output_mode; }
void ParsedConfig::set_run_mode(RunMode mode) { _run_mode = mode; }
void ParsedConfig::set_output_mode(OutputMode mode) { _output_mode = mode; }

void ParsedConfig::set_output_mode(std::string mode) {
  if (mode == "json") {
    _output_mode = OutputMode::JSON;
  } else if (mode == "conky") {
    _output_mode = OutputMode::CONKY;
  } else {
    std ::cerr << "Error: invalid output mode `" << mode << "`" << std::endl;
  }
}
void ParsedConfig::set_run_mode(std::string mode) {
  if (mode == "persistent") {
    _run_mode = RunMode::PERSISTENT;
  } else if (mode == "run_once") {
    _run_mode = RunMode::RUN_ONCE;
  } else {
    std ::cerr << "Error: invalid run mode `" << mode << "`" << std::endl;
  }
}
void ParsedConfig::sleep() {
  sleep_until += get_polling_interval<std::chrono::milliseconds>();
  std::this_thread::sleep_until(sleep_until);
}

void ParsedConfig::configure_renderer() {
  for (MetricsContext& task : tasks)
    // 1. Select and Configure the pipeline ONCE
    switch (_output_mode) {
      case OutputMode::JSON: {
        this->active_pipeline = configure_json_pipeline(task.settings);
        break;
      }
      case OutputMode::CONKY: {
        this->active_pipeline = configure_conky_pipeline(task.settings);
        break;
      }
      default:
        std::cerr << "Invalid output type" << std::endl;
        exit(1);
    }
}

int ParsedConfig::initialize(std::list<SystemMetrics>& tasks) {
  this->configure_renderer();

  if (this->tasks.empty()) {
    std::cerr << "Initialization failed, no valid tasks to run." << std::endl;
    return 1;
  }
  /* Perform these steps only once */
  for (MetricsContext& task : this->tasks) {
    SystemMetrics& new_task = tasks.emplace_back(task);

    DEBUG_PTR("Initialize context", task);
    DEBUG_PTR("New task", new_task);
    if (new_task.read_data() != 0) {
      std::cerr << "Warning: Failed to read initial data for task."
                << std::endl;
      // Optional: tasks.pop_back(); // Remove failed task if strict
    }
  }

  sleep_until = std::chrono::steady_clock::now();

  for (SystemMetrics& task : tasks) {
    DEBUG_PTR("Initialize Task", task);
    for (std::unique_ptr<IPollingTask>& polling_task : task.polling_tasks) {
      DEBUG_PTR("Polling task address", polling_task);
      polling_task->take_snapshot_1();
    }
  }
  return 0;
}
void ParsedConfig::done(std::list<SystemMetrics>& result) {
  // 2. Simply execute it.
  // The 'active_pipeline' already knows what to do and holds the necessary
  // settings/serializer.

  if (this->active_pipeline) {
    DEBUG_PTR("Active pipeline", this->active_pipeline);
    DEBUG_PTR("Active pipeline result", result);
    this->active_pipeline(result);
  }
}
