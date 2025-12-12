// batteryinfo.hpp
#pragma once
#include "pcn.hpp"

// Configuration (from Lua)
struct BatteryConfig {
  std::string name;
  std::string path;
};

// Runtime Data (from /sys)
struct BatteryStatus {
  std::string name;
  int percentage = 0;
  std::string status = "Unknown";  // Charging, Discharging, Full
  std::string path;
  bool present = false;
};
