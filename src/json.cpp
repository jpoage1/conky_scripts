// json.cpp

#include "data.hpp"
#include "json_definitions.hpp"
#include "log.hpp"
#include "metrics.hpp"
#include "parser.hpp"
#include "polling.hpp"
#include "runner.hpp"

int main(int argc, char* argv[]) {
  // 1. Call the parser (handles initial checks and processing)
  ParsedConfig config = parse_arguments(argc, argv);
  std::list<SystemMetrics> tasks;
  config.initialize(tasks);

  do {
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
