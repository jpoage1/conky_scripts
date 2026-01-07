// output_mode_json.hpp
#ifndef OUTPUT_MODE_JSON_HPP
#define OUTPUT_MODE_JSON_HPP

#include "types.hpp"

namespace telemetry {

struct RunnerContext;

class MetricsSettings;

void register_json_pipeline();
OutputPipeline configure_json_pipeline(const MetricSettings &settings);
int json_main(const RunnerContext &context);

}; // namespace telemetry

#endif
