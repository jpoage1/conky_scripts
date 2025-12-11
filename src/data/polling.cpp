// polling.cpp
#include "polling.hpp"

void IPollingTask::set_timestamp() {
  timestamp = std::chrono::steady_clock::now();
}
void IPollingTask::set_delta_time() {
  auto t2_timestamp = std::chrono::steady_clock::now();
  std::chrono::duration<double> time_delta = t2_timestamp - timestamp;
  timestamp = t2_timestamp;
  time_delta_seconds = time_delta.count();
}
