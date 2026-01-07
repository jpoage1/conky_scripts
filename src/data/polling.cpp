// polling.cpp
#include "polling.hpp"

namespace telemetry {

void IPollingTask::set_timestamp() {
  timestamp = std::chrono::steady_clock::now();
}

void IPollingTask::set_delta_time() {
  auto current_timestamp = std::chrono::steady_clock::now();
  std::chrono::duration<double> time_delta = current_timestamp - timestamp;
  timestamp = current_timestamp;
  time_delta_seconds = time_delta.count();
}

}; // namespace telemetry
