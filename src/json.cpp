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
  std::chrono::time_point<std::chrono::steady_clock> t1_timestamp;
  config.initialize(tasks, t1_timestamp);

  do {
    config.sleep();
    for (SystemMetrics& task : tasks) {
      //   std::cerr << "Running task" << std::endl;
      task.read_data();

      //   std::cerr << "Done running task" << std::endl;
    }

    // Get T2 snapshot
    auto t2_timestamp = std::chrono::steady_clock::now();

    std::chrono::duration<double> time_delta = t2_timestamp - t1_timestamp;
    t1_timestamp = t2_timestamp;
    for (SystemMetrics& task : tasks) {
      //   std::cerr << "TASK: &task = " << &task
      //             << ", disk_io size = " << task.disk_io.size()
      //             << ", &disk_io = " << &task.disk_io << std::endl;
      for (std::unique_ptr<IPollingTask>& polling_task : task.polling_tasks) {
        polling_task->take_snapshot_2();
        polling_task->calculate(time_delta.count());
        polling_task->commit();
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
    // Print the result or whatever
    config.done(tasks);
    // std::cerr << "Done dumping" << std::endl;

  } while (config.run_mode(RunMode::PERSISTENT));
  return 0;
}
