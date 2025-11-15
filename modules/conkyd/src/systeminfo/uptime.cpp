// uptime.cpp
#include "uptime.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>

#include "data_local.h"
#include "data_ssh.h"

std::istream& LocalDataStreams::get_uptime_stream() {
  reset_stream(uptime, "/proc/uptime");
  return uptime;
}
std::istream& ProcDataStreams::get_uptime_stream() {
  std::string uptime_data = execute_ssh_command("cat /proc/uptime");
  uptime.str(uptime_data);
  rewind(uptime, "uptime");
  return uptime;
}
std::string get_uptime(std::istream& input_stream) {
  double uptime_seconds;
  input_stream >> uptime_seconds;
  int days = uptime_seconds / 86400;
  int hours = ((int)uptime_seconds % 86400) / 3600;
  int minutes = ((int)uptime_seconds % 3600) / 60;
  return std::to_string(days) + "d " + std::to_string(hours) + "h " +
         std::to_string(minutes) + "m";
}
