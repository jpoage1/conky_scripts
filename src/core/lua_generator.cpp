// src/core/lua_generator.cpp
#include "lua_generator.hpp"
#include "default_config_lua.hpp"
#include <sstream>

namespace telemetry {
using string = std::string;
using generator = LuaConfigGenerator;
generator::LuaConfigGenerator(int level) : indentation_level(level) {}
generator::LuaConfigGenerator(std::string name, int level)
    : name(name), indentation_level(level) {}

void indent(int diff) {
  indentation_level += diff;
  indent();
}
void indent() {
  for (int i = 0; i < indentation_level; i++)
    lua << tab; // tab = "  ";, for example
}

void generator::lua_bool(string name, bool default_value) {
  indent();
  this->lua << name << " = " << default_value << std::endl;
}
u void generator::lua_int(string name, int default_value) {
  indent();
  this->lua << name << " = " << default_value << std::endl;
}
void generator::lua_double(string name, double default_value) {
  indent();
  this->lua << name << " = " << default_value << std::endl;
}
void generator::lua_string(string name, string default_value) {
  indent();
  this->lua << name << " = \"" << default_value << "\"" << std::endl;
}
void generator::lua_mklist(string name, string list) {
  indent(1);
  lua << name << " = {" << std::endl;
  lua << list;
  indent();
  lua << "}" << std::endl;
}
string generator::str() {
  stringstream lua_str = lua_list(name, lua.str());
  return lua_str.str();
}

} // namespace telemetry
