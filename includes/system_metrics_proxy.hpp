#ifndef SYSTEM_METRICS_PROXY_HPP
#define SYSTEM_METRICS_PROXY_HPP
#include <nlohmann/json.hpp>

#include "metrics.hpp"

class SystemMetricsProxy {
public:
  SystemMetricsProxy() = default;
  virtual ~SystemMetricsProxy() = default;

  // Virtual sink for pipeline data
  virtual void updateData(const nlohmann::json &data) = 0;
};
#endif
