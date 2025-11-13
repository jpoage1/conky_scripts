// waybar_types.hpp
#pragma once

#include <set>
#include <functional>
#include <exception>

#include "metrics.hpp"
#include "types.h"

using callback = std::function<int(const std::string&, CombinedMetrics&)>;

struct MetricResult {
  std::string source_name;
  std::string device_file;
  CombinedMetrics metrics;    // Will be empty on error
  std::string error_message;  // Will be empty on success
  bool success;
  std::set<std::string> specific_interfaces;

  callback get_metrics_callback;

  void set_callback(callback _get_metrics_callback) {
    get_metrics_callback = _get_metrics_callback;
  }
  void run() {
    if (!get_metrics_callback) {
        error_message = "Callback not set.";
        success = false;
        return;
    }
    try {
        // --- 1. EXPECTED ERRORS (Return Code) ---
        // Try to run the callback normally
        int return_code = get_metrics_callback(device_file, metrics);

        if (return_code == 0) {
            success = true;
        } else {
            success = false;
            if (error_message.empty()) {
                error_message = "Callback failed with code " + std::to_string(return_code);
            }
        }

    } catch (const std::exception& e) {
        // --- 2. UNEXPECTED ERRORS (Exception) ---
        // The callback threw an exception (e.g., std::runtime_error)
        success = false;
        error_message = "Callback threw an exception: " + std::string(e.what());

    } catch (...) {
        // --- 3. UNKNOWN ERRORS (Anything Else) ---
        // The callback threw something that wasn't an exception
        success = false;
        error_message = "Callback threw an unknown non-standard exception.";
    }
  }
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
