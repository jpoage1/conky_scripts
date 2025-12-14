// uptime.hpp
#pragma once

#include "pcn.hpp"

struct Time {
  int days, hours, minutes, seconds;
  double raw;

  // Default constructor
  Time() = default;

  // Parameterized constructor
  Time(int d, int h, int m, int s, double r)
      : days(d), hours(h), minutes(m), seconds(s), raw(r) {}

  // Format: "1d 4h 30m"
  std::string to_str() const {
    return std::to_string(days) + "d " + std::to_string(hours) + "h " +
           std::to_string(minutes) + "m";
  }

  // Format: "04:30:15"
  std::string to_clock_str() const {
    char buffer[32];
    std::sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
    return std::string(buffer);
  }
};

Time get_uptime(std::istream&);
