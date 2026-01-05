// json_pipeline.cpp
#ifndef JSON_PIPELINE_HPP
#define JSON_PIPELINE_HPP

// Factory functions: They take settings and return a runnable function
OutputPipeline configure_conky_pipeline(const MetricSettings& settings);

// --- JSON FACTORY ---
OutputPipeline configure_json_pipeline(const MetricSettings& settings);

OutputPipeline configure_widget_pipeline(const MetricSettings& settings);

#endif
