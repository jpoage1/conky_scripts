// runner.cpp
#include "runner.hpp"

#include "metrics.hpp"

void MetricsContext::set_callback(callback _get_metrics_callback) {
  //   provider = std::move(_provider);
  get_metrics_callback = _get_metrics_callback;
}
void MetricsContext::run() {
  if (!get_metrics_callback) {
    error_message = "Callback not set.";
    success = false;
    return;
  }
  if (!provider) {
    std::cerr << "Error: Provider not initialized!" << std::endl;
    success = false;
    return;
  }
  try {
    // std::cerr << "Calling the metrics callback" << std::endl;
    int return_code = get_metrics_callback(provider, device_file, metrics);

    if (return_code == 0) {
      success = true;
    } else {
      success = false;
      if (error_message.empty()) {
        error_message =
            "Callback failed with code " + std::to_string(return_code);
      }
    }

  } catch (const std::exception& e) {
    success = false;
    error_message = "Callback threw an exception: " + std::string(e.what());

  } catch (...) {
    success = false;
    error_message = "Callback threw an unknown non-standard exception.";
  }
}
