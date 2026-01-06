// output_mode_json.hpp
#ifndef OUTPUT_MODE_JSON_HPP
#define OUTPUT_MODE_JSON_HPP

struct RunnerContext;

OutputPipeline configure_json_pipeline(const MetricSettings& settings);
int json_main(const RunnerContext& context);

#endif
