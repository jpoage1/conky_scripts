// json.cpp

#include "data.hpp"
#include "json_definitions.hpp"
#include "metrics.hpp"
#include "parser.hpp"
#include "polling.hpp"
#include "runner.hpp"

int main(int argc, char* argv[]) {
  // 1. Call the parser (handles initial checks and processing)
  ParsedConfig config = parse_arguments(argc, argv);
  std::vector<SystemMetrics> tasks;

  if (config.tasks.empty()) {
    std::cerr << "Initialization failed, no valid tasks to run." << std::endl;
    return 1;
  }
  /* Perform these steps only once */
  for (MetricsContext& task : config.tasks) {
    // if (!task.provider) continue;
    // SystemMetrics metrics(task);
    // tasks.push_back(std::move(metrics));
    tasks.emplace_back(task);
  }
  for (SystemMetrics& task : tasks) {
    //   std::cerr << "Running task" << std::endl;
    task.read_data();

    //   std::cerr << "Done running task" << std::endl;
  }

  // A. Get T1 snapshot
  auto t1_timestamp = std::chrono::steady_clock::now();
  for (SystemMetrics& task : tasks) {
    for (std::unique_ptr<IPollingTask>& polling_task : task.polling_tasks) {
      // std::cerr << "Taking snapshot" << std::endl;
      polling_task->take_snapshot_1();
    }
  }
  do {
    for (SystemMetrics& task : tasks) {
      //   std::cerr << "Running task" << std::endl;
      task.read_data();

      //   std::cerr << "Done running task" << std::endl;
    }

    std::this_thread::sleep_for(
        config.get_pooling_interval<std::chrono::milliseconds>());
    // C. Get T2 snapshot
    auto t2_timestamp = std::chrono::steady_clock::now();

    std::chrono::duration<double> time_delta = t2_timestamp - t1_timestamp;
    for (SystemMetrics& task : tasks) {
      //   std::cerr << "TASK: &task = " << &task
      //             << ", disk_io size = " << task.disk_io.size()
      //             << ", &disk_io = " << &task.disk_io << std::endl;
      for (std::unique_ptr<IPollingTask>& polling_task : task.polling_tasks) {
        polling_task->take_snapshot_2();
        polling_task->calculate(time_delta.count());
        // polling_task->commit(); // unused/dead code
        // std::cerr << "AFTER COMMIT: disk_io size: " << task.disk_io.size()
        //           << std::endl;
      }

      // refresh data after polling
      //   task.complete(); // unused/dead code
      //   std::cerr << "AFTER COMPLETE: disk_io size: " << task.disk_io.size()
      //             << std::endl;

      // Cleanup ssh session
      task.provider->finally();  // unused/dead code
      //   std::cerr << "AFTER FINALLY: disk_io size: " << task.disk_io.size()
      //             << std::endl;
    }
    // for (SystemMetrics& task : tasks) {
    // //   std::cerr << "BEFORE SERIALIZATION: disk_io size: " <<
    // task.disk_io.size()
    // //             << std::endl;
    //   for (const auto& [name, stats] : task.disk_io) {
    //     std::cerr << "  - " << name << ": device_name='" << stats.device_name
    //               << "', read=" << stats.read_bytes_per_sec
    //               << ", write=" << stats.write_bytes_per_sec << std::endl;
    //   }
    // }
    // Print the result or whatever
    config.done(tasks);
    // std::cerr << "Done dumping" << std::endl;

    if (!config.run_mode(RunMode::RUN_ONCE)) {
      std::this_thread::sleep_for(
          config.get_pooling_interval<std::chrono::milliseconds>());
    }
    // std::cerr << "Done sleeping" << std::endl;
  } while (config.run_mode(RunMode::PERSISTENT));
  return 0;
}
