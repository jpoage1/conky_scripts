// json_pipeline.cpp
#include "cli_parser.hpp"
#include "context.hpp"
#include "data_local.hpp"
#include "data_ssh.hpp"
#include "log.hpp"
#include "metric_settings.hpp"
#include "output_mode_json.hpp"
#include "parsed_config.hpp"
#include "polling.hpp"
#include "types.hpp"
#include "uptime.hpp"

// First:
#include "json_definitions.hpp"

// Last:
#include "json_serializer.hpp"

namespace telemetry {

void register_json_pipeline() {
  PipelineEntry pipeline{"json", configure_json_pipeline, json_main, nullptr};
  ParsedConfig::register_pipeline(pipeline);
}

// --- JSON FACTORY ---
OutputPipeline configure_json_pipeline(const MetricSettings &settings) {
  // 1. Create the Serializer with the specific settings
  // We use a shared_ptr so the lambda can capture it cheaply and keep it alive
  auto serializer = std::make_shared<JsonSerializer>(settings);

  // 2. Return the executable function
  return [serializer](const std::list<SystemMetrics> &result) {
    nlohmann::json output_json = nlohmann::json::array();

    // The serializer logic is already baked in; no 'if' checks needed here
    for (const auto &metrics : result) {
      DEBUG_PTR("configure_json_pipeline lambda Metrics", metrics);
      output_json.push_back(serializer->serialize(metrics));
    }

    std::cout << output_json.dump() << std::endl;
  };
}

}; // namespace telemetry
