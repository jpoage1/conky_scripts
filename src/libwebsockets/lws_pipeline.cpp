
// src/lws/lws_pipeline.cpp
#include "cli_parser.hpp"
#include "configuration_builder.hpp"
#include "controller.hpp"
#include "json_serializer.hpp"
#include "libwebsockets.hpp"
#include "lws_proxy.hpp"
#include "parsed_config.hpp"
#include "qt.hpp"
#include "types.hpp"

namespace libwebsockets {

telemetry::PipelineFactory
lws_stream_factory(std::shared_ptr<SystemMetricsLwsProxy> proxy) {
  return [proxy](const telemetry::MetricSettings &settings)
             -> telemetry::OutputPipeline {
    auto serializer = std::make_shared<telemetry::JsonSerializer>(settings);
    return [serializer,
            proxy](const std::list<telemetry::SystemMetrics> &results) {
      if (!proxy)
        return;

      nlohmann::json json_array = nlohmann::json::array();
      for (const auto &m : results) {
        json_array.push_back(serializer->serialize(m));
      }
      proxy->updateData(json_array);
    };
  };
}

void register_libwebsockets_pipeline() {
  auto proxy = std::make_shared<SystemMetricsLwsProxy>();
  // Register the entry point with lws_main and our custom factory
  telemetry::PipelineEntry entry{"lws", lws_stream_factory(proxy), lws_main,
                                 proxy};
  telemetry::ParsedConfig::register_pipeline(entry);
}
}; // namespace libwebsockets
