// context.hpp
#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "context.hpp"
#include "diskstat.hpp"
#include "metric_settings.hpp"
#include "metrics.hpp"
#include "pcn.hpp"
#include "provider.hpp"
// struct MetricSettings;

namespace telemetry {
class SystemMetrics;

using callback = std::function<int(DataStreamProviderPtr &, const std::string &,
                                   SystemMetrics &)>;
using DiskStatConfig = std::set<DiskStatSettings>;

struct MetricsContext {
  MetricSettings settings;

  std::string source_name;
  std::string device_file = "";

  std::string user;
  std::string host;
  std::string port;
  std::string key;
  DataStreamProviders provider;
  std::string error_message;
  bool success;
  std::set<std::string> interfaces;
  std::vector<std::string> filesystems;
  std::vector<std::string> io_devices;
  DiskStatConfig disk_stat_config;

  MetricsContext(MetricsContext &&) noexcept = default;
  MetricsContext &operator=(MetricsContext &&) noexcept = default;

  MetricsContext(const MetricsContext &) = delete;
  MetricsContext &operator=(const MetricsContext &) = delete;

  MetricsContext() = default;
};

}; // namespace telemetry
#endif
