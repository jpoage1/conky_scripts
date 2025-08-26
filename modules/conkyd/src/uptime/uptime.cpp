#include "uptime.h"

#include <fstream>
#include <iomanip>
#include <iostream>

std::string get_uptime(std::istream &input_stream) {
  double uptime_seconds;
  input_stream >> uptime_seconds;
  int days = uptime_seconds / 86400;
  int hours = ((int)uptime_seconds % 86400) / 3600;
  int minutes = ((int)uptime_seconds % 3600) / 60;
  return std::to_string(days) + "d " + std::to_string(hours) + "h " +
         std::to_string(minutes) + "m";
}
