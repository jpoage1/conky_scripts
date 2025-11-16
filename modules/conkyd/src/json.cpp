// json.cpp
#include "nlohmann/json.hpp"

#include <chrono>  // For seconds
#include <iostream>
#include <thread>  // For sleep_for
#include <vector>

#include "data.h"
#include "json_definitions.hpp"  // JSON serialization macros
#include "metrics.hpp"
#include "parser.hpp"  // Shared parser (handles argc check now)
#include "runner.hpp"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
  // 1. Call the parser (handles initial checks and processing)
  ParsedConfig config = parse_arguments(argc, argv);

  if (config.tasks.empty()) {
    std::cerr << "Initialization failed, no valid tasks to run." << std::endl;
    return 1;
  }
  do {
    for (MetricsContext& task : config.tasks) {
      //   std::cerr << "Running task" << std::endl;
      task.run();
      //   std::cerr << "Done running task" << std::endl;
    }

    // A. Get T1 snapshot
    auto t1_timestamp = std::chrono::steady_clock::now();
    for (MetricsContext& task : config.tasks) {
      if (task.provider) {
        for (auto& polled_task : task.metrics.polled) {
          polled_task->take_snapshot_1();
        }
      } else {
        std::cerr << "Provider context is empty" << std::endl;
      }
    }
    std::this_thread::sleep_for(
        config.get_pooling_interval<std::chrono::milliseconds>());
    // C. Get T2 snapshot
    auto t2_timestamp = std::chrono::steady_clock::now();

    for (MetricsContext& task : config.tasks) {
      for (auto& polled_task : task.metrics.polled) {
        polled_task->take_snapshot_2();
      }
    }

    // D. Calculate
    std::chrono::duration<double> time_delta = t2_timestamp - t1_timestamp;
    for (MetricsContext& task : config.tasks) {
      for (auto& polled_task : task.metrics.polled) {
        polled_task->calculate(time_delta.count());
      }
    }

    config.done();
    // std::cerr << "Done dumping" << std::endl;

    if (!config.run_mode(RunMode::RUN_ONCE)) {
      std::this_thread::sleep_for(
          config.get_pooling_interval<std::chrono::milliseconds>());
    }
    // std::cerr << "Done sleeping" << std::endl;
  } while (config.run_mode(RunMode::PERSISTENT));
  return 0;
}
