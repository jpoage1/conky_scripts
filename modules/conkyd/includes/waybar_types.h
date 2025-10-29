// waybar_types.hpp
#pragma once

#include <set>

#include "metrics.hpp"
#include "types.h"

struct MetricResult {
  std::string source_name;
  std::string device_file;
  CombinedMetrics metrics;    // Will be empty on error
  std::string error_message;  // Will be empty on success
  bool success;
  std::set<std::string> specific_interfaces;
};

struct FormattedSize {
  std::string text;
  std::string color;

  std::string formatted(TargetFormat target) const {
    if (target == TargetFormat::WAYBAR) {
      return "<span foreground='#" + color + "'>" + text + "</span>";
    }

    return text;
  }
};
