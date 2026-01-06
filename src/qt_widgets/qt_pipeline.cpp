// src/qt/pipeline.cpp
#include "cli_parser.hpp"
#include "configuration_builder.hpp"
#include "controller.hpp"
#include "json_serializer.hpp"
#include "parsed_config.hpp"
#include "qt.hpp"
#include "system_metrics_qt_proxy.hpp"
#include "types.hpp"

void register_qt_pipeline() {
  auto proxy = std::make_shared<SystemMetricsQtProxy>();
  PipelineEntry pipeline{"qt", qt_widget_factory(proxy.get()), qt_main, proxy};
  ParsedConfig::register_pipeline(pipeline);
}

PipelineFactory qt_widget_factory(SystemMetricsQtProxy* proxy) {
  return [proxy](const MetricSettings& settings) -> OutputPipeline {
    auto serializer = std::make_shared<JsonSerializer>(settings);
    return [serializer, proxy](const std::list<SystemMetrics>& results) {
      if (!proxy) return;
      nlohmann::json json_data = nlohmann::json::array();
      for (const auto& m : results) {
        json_data.push_back(serializer->serialize(m));
      }
      // std::cout << json_data.dump() ;
      proxy->updateData(json_data);
    };
  };
}
