// uptime.cpp
#include "uptime.hpp"

#include "data_local.hpp"
#include "data_ssh.hpp"

namespace telemetry {

std::istream &LocalDataStreams::get_uptime_stream() {
  return create_stream_from_file(uptime, "/proc/uptime");
}
std::istream &ProcDataStreams::get_uptime_stream() {
  return create_stream_from_command(uptime, "cat /proc/uptime");
}
Time get_uptime(std::istream &input_stream) {
  double uptime_seconds;
  if (input_stream >> uptime_seconds) {
    int days = uptime_seconds / 86400;
    int hours = ((int)uptime_seconds % 86400) / 3600;
    int minutes = ((int)uptime_seconds % 3600) / 60;
    int seconds = (int)uptime_seconds % 60;
    return {days, hours, minutes, seconds, uptime_seconds};
  }
  return {0, 0, 0, 0, 0};
}

// Default constructor
Time::Time() = default;

// Parameterized constructor
Time::Time(int d, int h, int m, int s, double r)
    : days(d), hours(h), minutes(m), seconds(s), raw(r) {}

// Format: "1d 4h 30m"
std::string Time::to_str() const {
  return std::to_string(days) + "d " + std::to_string(hours) + "h " +
         std::to_string(minutes) + "m";
}

// Format: "04:30:15"
std::string Time::to_clock_str() const {
  char buffer[32];
  std::sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
  return std::string(buffer);
}
}; // namespace telemetry
