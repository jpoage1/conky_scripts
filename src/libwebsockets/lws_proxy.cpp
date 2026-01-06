// src/libwebsockets/lws_proxy.cpp
#include "lws_proxy.hpp"
#include "system_metrics_proxy.hpp"
#include <mutex>
#include <shared_mutex>
#include <string>

void SystemMetricsLwsProxy::updateData(const nlohmann::json &data) {
  std::unique_lock lock(mutex);
  current_json_str = data.dump();
  has_new_data = true;
}

std::string SystemMetricsLwsProxy::consume() {
  std::unique_lock lock(mutex);
  has_new_data = false;
  return current_json_str;
}

bool SystemMetricsLwsProxy::ready() const {
  std::shared_lock lock(mutex);
  return has_new_data;
}
