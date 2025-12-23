// parsed_config.cpp
#include "cli_parser.hpp"
#include "config_types.hpp"
#include "conky_output.hpp"
#include "context.hpp"
#include "log.hpp"
#include "pipeline.hpp"
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
  sleep_until = std::chrono::steady_clock::now();
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
    for (std::unique_ptr<IPollingTask>& polling_task : new_task.polling_tasks) {
      DEBUG_PTR("Polling task address", polling_task);
      polling_task->take_initial_snapshot();
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
bool ParsedConfig::reload_if_changed(std::list<SystemMetrics>& active_tasks) {
  if (config_path.empty()) return false;

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

    } catch (const std::exception& e) {
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
