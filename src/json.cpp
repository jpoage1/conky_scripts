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
  std::chrono::time_point<std::chrono::steady_clock> t1_timestamp;
  config.initialize(tasks, t1_timestamp);

  spdlog::set_level(spdlog::level::debug);

  do {
    config.sleep();
    for (SystemMetrics& task : tasks) {
      SPDLOG_DEBUG("Running task");
      task.read_data();
      SPDLOG_DEBUG("Done running task");
    }

    // Get T2 snapshot
    auto t2_timestamp = std::chrono::steady_clock::now();

    std::chrono::duration<double> time_delta = t2_timestamp - t1_timestamp;
    t1_timestamp = t2_timestamp;
    for (SystemMetrics& task : tasks) {
      std::stringstream ss;
      ss << &task;
      SPDLOG_DEBUG("Task address: {}", ss.str());
      for (std::unique_ptr<IPollingTask>& polling_task : task.polling_tasks) {
        std::stringstream ss;
        ss << &polling_task << " " << polling_task.get();
        SPDLOG_DEBUG("Polling task address: {}", ss.str());
        polling_task->take_snapshot_2();
        polling_task->calculate(time_delta.count());
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
