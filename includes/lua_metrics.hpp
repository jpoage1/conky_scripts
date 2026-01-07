#ifndef LUA_METRICS_HPP
#define LUA_METRICS_HPP

#include "lua_generator.hpp"
#include "metric_settings.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sol/sol.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace telemetry {
struct LuaConfigGenerator;
using Generator = LuaConfigGenerator;

struct LuaFeatures : public Features {
  std::string serialize(unsigned const int indentation_level = 0) const;
  void deserialize(sol::table features);
};

struct LuaMetricSettings : public MetricSettings {
  std::string serialize(unsigned indentation_level = 0) const;
  void deserialize(sol::table settings);
};

struct LuaMetricsConfig : public MetricsConfig {
  std::string serialize() const;
  void deserialize(sol::table config);
};
}; // namespace telemetry

#endif
