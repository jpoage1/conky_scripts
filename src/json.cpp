
#include "cli_parser.hpp"
#include "config_types.hpp"
#include "context.hpp"
#include "json_definitions.hpp"
#include "log.hpp"
#include "lua_parser.hpp"
#include "metrics.hpp"
#include "polling.hpp"
#include "stream_provider.hpp"

int main(int argc, char* argv[]) {
  // Phase 1: Parse Text
  ProgramOptions options = parse_cli(argc, argv);

  ParsedConfig config;  // Your existing runtime controller

  // Handle Exclusive Global Config
  if (options.global_config_file.has_value()) {
    config = load_lua_config(options.global_config_file.value());
    // Early exit logic handles here
  } else {
    // Phase 2: Build Contexts
    if (options.persistent) {
      config.set_run_mode(RunMode::PERSISTENT);
    }

    for (const auto& cmd : options.commands) {
      MetricsContext context;

      if (cmd.type == CommandType::LOCAL) {
        // Logic moved from process_command
        context.provider = DataStreamProviders::LocalDataStream;
        context.device_file = cmd.config_path;
        // Validate file existence here
      } else if (cmd.type == CommandType::SETTINGS) {
        context = load_lua_settings(cmd.config_path);
      }

      config.tasks.push_back(std::move(context));
    }
  }

  // Phase 3: Execute
  std::list<SystemMetrics> tasks;
  config.initialize(tasks);

  do {
    if (config.reload_if_changed(tasks)) {
      continue;
    }
    config.sleep();
    for (SystemMetrics& task : tasks) {
      SPDLOG_DEBUG("Running task");
      task.read_data();
      SPDLOG_DEBUG("Done running task");
    }

    for (SystemMetrics& task : tasks) {
      DEBUG_PTR("main SystemMetrics task address", task);
      for (std::unique_ptr<IPollingTask>& polling_task : task.polling_tasks) {
        DEBUG_PTR("Polling task address", polling_task);
        polling_task->take_snapshot_2();
        polling_task->calculate();
        polling_task->commit();
      }

      // refresh data after polling
      //   task.complete(); // unused/dead code

      // Cleanup ssh session
      task.provider->finally();
    }
    // Print the result or whatever
    SPDLOG_DEBUG("config.done()");
    config.done(tasks);

  } while (config.run_mode(RunMode::PERSISTENT));
  return 0;
}
