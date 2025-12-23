// json_serializer.hpp
#ifndef JSON_SERIALIZER_HPP
#define JSON_SERIALIZER_HPP

#include <functional>
#include <nlohmann/json.hpp>
#include <vector>

#include "batteryinfo.hpp"
#include "corestat.hpp"
#include "diskstat.hpp"
#include "filesystems.hpp"
#include "log.hpp"
#include "meminfo.hpp"
#include "metrics.hpp"
#include "networkstats.hpp"
#include "processinfo.hpp"
#include "stream_provider.hpp"
#include "uptime.hpp"

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
