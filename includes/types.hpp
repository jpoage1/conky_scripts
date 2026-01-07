// types.hpp
#ifndef TYPES_HPP
#define TYPES_HPP

#include "pcn.hpp"

namespace telemetry {

struct RunnerContext;
struct PipelineEntry;

class SystemMetrics;
class MetricSettings;
class Controller;
class SystemMetricsProxy;
class SystemMetricsQtProxy;

using OutputMode = std::string;

using ControllerPtr = std::unique_ptr<Controller>;
using SystemMetricsProxyPtr = std::shared_ptr<SystemMetricsProxy>;
using SystemMetricsQtProxyPtr = std::shared_ptr<SystemMetricsQtProxy>;

using OutputPipeline = std::function<void(const std::list<SystemMetrics> &)>;
using PipelineFactory = std::function<OutputPipeline(const MetricSettings &)>;
using PipelineRegistry = std::map<OutputMode, PipelineEntry>;
using MainOutput = std::function<int(const RunnerContext &)>;

enum class RunMode {
  RUN_ONCE,
  PERSISTENT,
};

struct PipelineEntry {
  OutputMode mode;
  PipelineFactory factory;
  MainOutput out = nullptr;
  SystemMetricsProxyPtr proxy = nullptr;
};

struct ActivePipeline {
  OutputPipeline processor;
  MainOutput entry_point = nullptr;
  SystemMetricsProxyPtr proxy = nullptr;
};

}; // namespace telemetry

#endif
