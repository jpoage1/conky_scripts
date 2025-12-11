// runner.hpp
#pragma once

#include "metrics.hpp"
#include "parser.hpp"
#include "pcn.hpp"
#include "provider.hpp"
#include "runner.hpp"
using callback = std::function<int(DataStreamProviderPtr&, const std::string&,
                                   SystemMetrics&)>;

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

  MetricsContext(MetricsContext&&) noexcept = default;
  MetricsContext& operator=(MetricsContext&&) noexcept = default;

  MetricsContext(const MetricsContext&) = delete;
  MetricsContext& operator=(const MetricsContext&) = delete;

  MetricsContext() = default;
};
