// lua_parser.hpp
#ifndef LUA_PARSER_HPP
#define LUA_PARSER_HPP

#include <sol/sol.hpp>

#include "pcn.hpp"

struct MetricsContext;

class ParsedConfig;

MetricsContext parse_settings(sol::table lua_settings);
ParsedConfig parse_config(sol::table lua_config);
MetricsContext load_lua_settings(const std::string& filename);
ParsedConfig load_lua_config(const std::string& filename);
sol::state load_lua_file(const std::string& filename);

#endif
