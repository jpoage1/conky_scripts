// types.hpp
#ifndef TYPES_HPP
#define TYPES_HPP
#include "pcn.hpp"

// Forward decl
class SystemMetrics;
class MetricSettings;

// The generic signature for ANY output strategy
using OutputPipeline = std::function<void(const std::list<SystemMetrics>&)>;
using PipelineFactory = std::function<OutputPipeline(const MetricSettings&)>;

enum class RunMode {
  RUN_ONCE,
  PERSISTENT,
};

enum class OutputMode {
  CONKY,
  JSON,
  WIDGETS,
};

#endif
