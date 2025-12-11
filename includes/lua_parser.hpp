// lua_parser.hpp
#pragma once

#include <sol/sol.hpp>

// #include "metrics.hpp"
#include "pcn.hpp"
struct MetricsContext;
struct ParsedConfig;

MetricsContext parse_settings(sol::table lua_settings);
ParsedConfig parse_config(sol::table lua_config);
MetricsContext load_lua_settings(const std::string& filename);
ParsedConfig load_lua_config(const std::string& filename);
sol::state load_lua_file(const std::string& filename);
