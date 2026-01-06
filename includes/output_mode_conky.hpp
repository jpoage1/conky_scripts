// output_mode_conky.hpp
#ifndef OUTPUT_MODE_CONKY_HPP
#define OUTPUT_MODE_CONKY_HPP

struct RunnerContext;
struct MetricSettings;

void register_conky_pipeline();
OutputPipeline configure_conky_pipeline(const MetricSettings &settings);
int conky_main(const RunnerContext &context);

#endif
