// uptime.hpp
#ifndef UPTIME_HPP
#define UPTIME_HPP

#include "pcn.hpp"

struct Time {
  int days, hours, minutes, seconds;
  double raw;

  // Default constructor
  Time();

  // Parameterized constructor
  Time(int d, int h, int m, int s, double r);

  // Format: "1d 4h 30m"
  std::string to_str() const;

  // Format: "04:30:15"
  std::string to_clock_str() const;
};

Time get_uptime(std::istream&);

#endif
