// json_serializer.hpp
#ifndef JSON_SERIALIZER_HPP
#define JSON_SERIALIZER_HPP

#include <functional>
#include <nlohmann/json.hpp>
#include <vector>

struct MetricSettings;

class SystemMetrics;

class JsonSerializer {
 public:
  using PipelineTask =
      std::function<void(nlohmann::json&, const SystemMetrics&)>;

 private:
  std::vector<PipelineTask> pipeline;

 public:
  // Constructor builds the pipeline ONCE based on settings
  JsonSerializer(const MetricSettings& settings);

  // The runtime function - No "if" checks here
  nlohmann::json serialize(const SystemMetrics& metrics) const;
};

#endif
