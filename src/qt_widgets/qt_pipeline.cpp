// src/qt/pipeline.cpp
#include "controller.hpp"
#include "configuration_builder.hpp"
#include "cli_parser.hpp"
#include "system_metrics_proxy.hpp"
#include "json_serializer.hpp"
#include "types.hpp"
#include "config_types.hpp"

PipelineFactory widget_factory(SystemMetricsProxy* proxy) {
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
