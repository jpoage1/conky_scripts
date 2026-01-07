// batteryinfo.hpp
#ifndef BATTERYINFO_HPP
#define BATTERYINFO_HPP

#include "batteryinfo.hpp"
#include <sol/sol.hpp>
#include <string>

namespace telemetry {

struct LuaBattery : public Battery {
  std::string serialize(unsigned indentation_level = 0) const;
  void deserialize(const sol::table &bat_item);
};

struct LuaBatteries : public Batteries {
  std::string serialize(unsigned indentation_level = 0) const;
  void deserialize(sol::table batteries_table);
};

}; // namespace telemetry
#endif
