// parsed_config.cpp
#include "parsed_config.hpp"

#include "cli_parser.hpp"
#include "conky_output.hpp"
#include "context.hpp"
#include "log.hpp"
#include "polling.hpp"
#include "qt.hpp"

namespace telemetry {

PipelineRegistry ParsedConfig::pipeline_registry = {};

bool ParsedConfig::run_mode(RunMode mode) const { return _run_mode == mode; }
bool ParsedConfig::output_mode(OutputMode mode) const {
  return _output_mode == mode;
}
RunMode ParsedConfig::run_mode() const { return _run_mode; }
OutputMode ParsedConfig::get_output_mode() const { return _output_mode; }

void ParsedConfig::set_run_mode(RunMode mode) { _run_mode = mode; }
void ParsedConfig::set_run_mode(std::string mode) {
  if (mode == "persistent") {
    _run_mode = RunMode::PERSISTENT;
  } else if (mode == "run_once") {
    _run_mode = RunMode::RUN_ONCE;
  } else {
    std ::cerr << "Error: invalid run mode `" << mode << "`" << std::endl;
  }
}
void ParsedConfig::set_output_mode(std::string mode) {
  if (pipeline_registry.find(mode) != pipeline_registry.end()) {
    SPDLOG_INFO("Output Mode: {}", mode);
    _output_mode = mode;
  } else {
    std::cerr << "Error: invalid output mode `" << mode << "`" << std::endl;
    show_output_modes();
  }
}
void ParsedConfig::show_output_modes() {
  // List available modes
  std::cerr << "Available: ";
  for (const auto &[name, pipeline] : pipeline_registry) {
    std::cerr << pipeline.mode << " ";
  }
  std::cerr << std::endl;
}
void ParsedConfig::sleep() {
  sleep_until += get_polling_interval<std::chrono::milliseconds>();
  std::this_thread::sleep_until(sleep_until);
}

void ParsedConfig::configure_renderer() {
  if (tasks.empty())
    return;

  auto it = pipeline_registry.find(_output_mode);
  if (it != pipeline_registry.end()) {
    // 1. Call the factory to create the PROCESSOR
    this->active_pipeline.processor =
        it->second.factory(tasks.front().settings);
    // 2. Store the ENTRY POINT
    this->active_pipeline.entry_point = it->second.out;
    this->active_pipeline.proxy = it->second.proxy;
  } else {
    std::cerr << "Fatal: Output mode '" << _output_mode << "' not registered."
              << std::endl;
    show_output_modes();
  }
}

void ParsedConfig::register_pipeline(const PipelineEntry pipeline) {
  SPDLOG_DEBUG("Registering pipeline: {}", pipeline.mode);
  ParsedConfig::pipeline_registry[pipeline.mode] = pipeline;
}

int ParsedConfig::initialize(std::list<SystemMetrics> &tasks) {
  this->configure_renderer();

  if (this->tasks.empty()) {
    std::cerr << "Initialization failed, no valid tasks to run." << std::endl;
    return 1;
  }
  sleep_until = std::chrono::steady_clock::now();
  /* Perform these steps only once */
  for (MetricsContext &task : this->tasks) {
    SystemMetrics &new_task = tasks.emplace_back(task);

    DEBUG_PTR("Initialize context", task);
    DEBUG_PTR("New task", new_task);
    if (new_task.read_data() != 0) {
      std::cerr << "Warning: Failed to read initial data for task."
                << std::endl;
      // Optional: tasks.pop_back(); // Remove failed task if strict
    }
    for (std::unique_ptr<IPollingTask> &polling_task : new_task.polling_tasks) {
      DEBUG_PTR("Polling task address", polling_task);
      polling_task->take_initial_snapshot();
    }
  }

  return 0;
}

void ParsedConfig::done(std::list<SystemMetrics> &result) {
  // Call the processor (the result of the factory)
  if (this->active_pipeline.processor) {
    this->active_pipeline.processor(result);
  }
}

bool ParsedConfig::reload_if_changed(std::list<SystemMetrics> &active_tasks) {
  if (config_path.empty())
    return false;

  std::error_code ec;
  auto current_time = std::filesystem::last_write_time(config_path, ec);

  if (ec) {
    SPDLOG_WARN("Could not check config file time: {}", ec.message());
    return false;
  }

  // If timestamp is newer, reload
  if (current_time > last_write_time) {
    SPDLOG_INFO("Config change detected. Reloading...");
    last_write_time = current_time;

    // 1. Load NEW config into a temporary object
    // We catch exceptions here to prevent crashing on bad syntax
    try {
      ParsedConfig new_config = load_lua_config(config_path);

      // 2. Clear OLD tasks (Destructors run, connections close)
      active_tasks.clear();

      // 3. Move NEW tasks into the active list
      // We iterate the 'tasks' inside the temporary 'new_config'
      // and initialize them into the running 'active_tasks' list.

      // Copy relevant settings over if needed (like run_mode)
      this->set_run_mode(new_config.run_mode());
      this->set_output_mode(new_config.get_output_mode());

      // Re-run initialization logic (creates tasks, opens streams)
      // Note: We need to temporarily swap the 'tasks' context
      // inside 'this' or just use the new_config's context.

      // Easier way: Steal the contexts from new_config
      this->tasks = std::move(new_config.tasks);

      // Re-run initialize logic on the main list
      this->initialize(active_tasks);

      SPDLOG_INFO("Hot reload complete.");
      return true;

    } catch (const std::exception &e) {
      SPDLOG_ERROR("Hot reload failed: {}", e.what());
      // We keep the old tasks running if the new config is broken
      return false;
    }
  }
  return false;
}

void ParsedConfig::set_filename(std::string filename) {
  config_path = filename;
  last_write_time = std::filesystem::last_write_time(filename);
}

int ParsedConfig::main(const RunnerContext &ctx) {
  // Check if the current mode has a GUI entry point (like qt_main)
  if (active_pipeline.entry_point) {
    return active_pipeline.entry_point(ctx);
  }
  return 1;
}
}; // namespace telemetry
