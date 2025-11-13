#include "nlohmann/json.hpp"

#include <iostream>
#include <vector>

#include <thread> // For sleep_for
#include <chrono> // For seconds


#include "json_definitions.hpp"  // JSON serialization macros
#include "parser.hpp"            // Shared parser (handles argc check now)
#include "waybar_types.h"
#include "metrics.hpp"
#include "data.h"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
  // 1. Call the parser (handles initial checks and processing)
  ParsedConfig config = parse_arguments(argc, argv);

  if (config.tasks.empty()) {
        std::cerr << "Initialization failed, no valid tasks to run." << std::endl;
        return 1;
    }
    do {

        for (MetricResult& task : config.tasks) {
            task.run();
        }

        // A. Get T1 snapshot
        auto t1_timestamp = std::chrono::steady_clock::now();
        for (MetricResult& task : config.tasks) {
            for (auto& polled_task : task.metrics.polled) {
                polled_task->take_snapshot_1();
            }
        }
        std::this_thread::sleep_for(config.get_pooling_interval<std::chrono::milliseconds>());
        // C. Get T2 snapshot
        auto t2_timestamp = std::chrono::steady_clock::now();

        for (MetricResult& task : config.tasks) {
            for (auto& polled_task : task.metrics.polled) {
                polled_task->take_snapshot_2();
            }
        }

        // D. Calculate
        std::chrono::duration<double> time_delta = t2_timestamp - t1_timestamp;
        for (MetricResult& task : config.tasks) {
            for (auto& polled_task : task.metrics.polled) {
                polled_task->calculate(time_delta.count());
            }
        }

        json output_json = config.tasks;
        std::cout << output_json.dump() << std::endl;

        if ( config.mode != RunMode::RUN_ONCE ) {
            std::this_thread::sleep_for(config.get_pooling_interval<std::chrono::milliseconds>());
        }
    } while  (config.mode == RunMode::PERSISTENT);
    return 0;
}
// int persistent_feed(int argc, char* argv[]) {
//     DataStreamProvider &provider;

//     // Get initial snapshot
//     PollingMetrics prev_metrics(provider);

//     while (true) {
//         // 1. Wait
//         std::this_thread::sleep_for(std::chrono::seconds(1));

//         // 2. Get current snapshot
//         PollingMetrics curr_metrics(provider);

//         // 3. Calculate dynamic rates
//         SystemMetrics metrics; // Holds all our data for this loop
//         std::chrono::duration<double> time_delta = curr_metrics.timestamp - prev_metrics.timestamp;

//         metrics.cores = calculate_cpu_usages(
//             prev_metrics.cpu_snapshots,
//             curr_metrics.cpu_snapshots
//         );
//         metrics.network_interfaces = calculate_network_rates(
//             prev_metrics.network_snapshots,
//             curr_metrics.network_snapshots,
//             time_delta.count()
//         );

//         // 4. Get all other static metrics...
//         // get_mem_usage(provider, metrics);
//         // get_swap_usage(provider, metrics);
//         // ...etc...

//         // 5. Serialize and print...
//         // json output_json = ... (build your MetricResult) ...
//         // std::cout << output_json.dump() << std::endl;

//         // 6. Set current as previous for next loop
//         prev_metrics = curr_metrics;
//     }
//     return 0; // Unreachable
// }
