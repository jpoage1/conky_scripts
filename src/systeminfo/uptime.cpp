// uptime.cpp
#include "uptime.hpp"

#include "data_local.hpp"
#include "data_ssh.hpp"

std::istream& LocalDataStreams::get_uptime_stream() {
  return create_stream_from_file(uptime, "/proc/uptime");
}
std::istream& ProcDataStreams::get_uptime_stream() {
  return create_stream_from_command(uptime, "cat /proc/uptime");
}
Time get_uptime(std::istream& input_stream) {
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
