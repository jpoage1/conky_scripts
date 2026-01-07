// includes/lws_proxy.hpp
#ifndef LWS_PROXY_HPP
#define LWS_PROXY_HPP

#include "system_metrics_proxy.hpp"
#include <shared_mutex>
#include <string>

namespace telemetry {

class SystemMetricsLwsProxy : public SystemMetricsProxy {
public:
  void updateData(const nlohmann::json &data) override;

  std::string consume();

  bool ready() const;

private:
  mutable std::shared_mutex mutex;
  std::string current_json_str;
  bool has_new_data = false;
};

}; // namespace telemetry
#endif
