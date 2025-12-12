// types.hpp
#pragma once
#include "pcn.hpp"

// Forward decl
class SystemMetrics;

// The generic signature for ANY output strategy
using OutputPipeline = std::function<void(const std::list<SystemMetrics>&)>;

enum class RunMode {
  RUN_ONCE,
  PERSISTENT,
};

enum class OutputMode {
  CONKY,
  JSON,
};
