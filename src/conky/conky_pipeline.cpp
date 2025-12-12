// conky_pipeline.cpp
#include "cli_parser.hpp"
#include "conky_output.hpp"
#include "context.hpp"
#include "log.hpp"
#include "polling.hpp"

// --- CONKY FACTORY ---
OutputPipeline configure_conky_pipeline(const MetricSettings& settings) {
  // 1. Capture settings by value for the lambda
  // (Or build a helper vector of print-functions like we did for JSON)
  return [settings](const std::list<SystemMetrics>& result) {
    for (const SystemMetrics& metrics : result) {
      DEBUG_PTR("configure_conky_pipeline lambda Metrics", metrics);
      print_metrics(metrics);
    }
    // for (const auto& m : result) {
    //   // This runs the loop, but using the captured settings
    //   if (settings.enable_sysinfo) {
    //     std::cout << "Node: " << m.node_name << std::endl;
    //   }
    //   if (settings.enable_cpu_temp) {
    //     std::cout << "Temp: " << m.cpu_temp_c << "C" << std::endl;
    //   }
    //   // ... etc ...
    // }
  };
}
