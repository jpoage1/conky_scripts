// runner.hpp
#pragma once

#include "metrics.hpp"
#include "pcn.hpp"
#include "types.h"

using callback = std::function<int(DataStreamProviderPtr&, const std::string&,
                                   CombinedMetrics&)>;

struct MetricsContext {
  std::string source_name;
  std::string device_file;
  CombinedMetrics metrics;    // Will be empty on error
  std::string error_message;  // Will be empty on success
  bool success;
  std::set<std::string> specific_interfaces;

  DataStreamProviderPtr provider;

  callback get_metrics_callback;

  //   // Prevent copying (unique_ptr can't be copied anyway)
  //   MetricsContext(const MetricsContext&) = delete;
  //   MetricsContext& operator=(const MetricsContext&) = delete;

  //   // Allow moving
  //   MetricsContext(MetricsContext&&) = default;
  //   MetricsContext& operator=(MetricsContext&&) = default;

  void set_callback(callback _get_metrics_callback);
  void run();
};
