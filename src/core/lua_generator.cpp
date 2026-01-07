#include "lua_generator.hpp"
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace telemetry {

using string = std::string;

LuaConfigGenerator::LuaConfigGenerator(unsigned int level)
    : indentation_level(level) {}
LuaConfigGenerator::LuaConfigGenerator(const std::string &name,
                                       unsigned int level)
    : name(name), indentation_level(level) {}

void LuaConfigGenerator::indent(int diff) {
  indentation_level += diff;
  indent();
}

void LuaConfigGenerator::indent() {
  for (unsigned int i = 0; i < indentation_level; i++)
    lua << tab;
}

void LuaConfigGenerator::lua_bool(const string &name,
                                  const bool &default_value) {
  indent();
  this->lua << name << " = " << default_value << std::endl;
}

void LuaConfigGenerator::lua_uint(const string &name,
                                  const unsigned int &default_value) {
  indent();
  this->lua << name << " = " << default_value << std::endl;
}

void LuaConfigGenerator::lua_int(const string &name, const int &default_value) {
  indent();
  this->lua << name << " = " << default_value << std::endl;
}

void LuaConfigGenerator::lua_double(const string &name,
                                    const double &default_value) {
  indent();
  this->lua << name << " = " << default_value << std::endl;
}

void LuaConfigGenerator::lua_string(const string &name,
                                    const string &default_value) {
  indent();
  this->lua << name << " = \"" << default_value << "\"" << std::endl;
}

void LuaConfigGenerator::lua_wrap(const std::string &text) {
  indent();
  lua << "{" << std::endl;
  lua << text;
  indent();
  lua << "}" << std::endl;
}

void LuaConfigGenerator::lua_vector(const std::vector<string> &values,
                                    const bool &indent_first) {
  if (indent_first)
    indent();
  lua << "{ " << std::endl;
  for (size_t i = 0; i < values.size(); ++i) {
    indent();
    lua << "\"" << values[i] << "\"" << (i == values.size() - 1 ? "" : ", ")
        << std::endl;
  }
  indent();
  lua << " }" << std::endl;
}

void LuaConfigGenerator::lua_vector(const string &name,
                                    const std::vector<string> &values) {
  indent();
  lua << name << " = ";
  lua_vector(values);
}

void LuaConfigGenerator::lua_raw_list(const std::vector<string> &entries) {
  std::stringstream ss;
  for (size_t i = 0; i < entries.size(); ++i) {
    ss << entries[i] << (i == entries.size() - 1 ? "" : ",\n");
  }
  lua_wrap(ss.str());
}

void LuaConfigGenerator::lua_raw_list(const string &name,
                                      const std::vector<string> &entries) {
  std::stringstream ss;
  for (size_t i = 0; i < entries.size(); ++i) {
    ss << entries[i] << (i == entries.size() - 1 ? "" : ",\n");
  }
  this->lua << lua_mklist(name, ss.str());
}

void LuaConfigGenerator::lua_list_entry(const std::string &entry) {
  lua << "\"" << entry << "\"" << ", ";
}

void LuaConfigGenerator::lua_list_raw_entry(const std::string &entry) {
  lua << entry << ", ";
}

const std::string LuaConfigGenerator::lua_mklist(const string &name,
                                                 const string &list) {
  indent(1);
  std::stringstream str;
  str << name << " = {" << std::endl;
  str << list;
  indent();
  str << "}" << std::endl;
  return str.str();
}
string LuaConfigGenerator::str() {
  if (name != "")
    return lua_mklist(name, lua.str());
  else {
    lua_wrap(lua.str());
    return lua.str();
  }
}
string LuaConfigGenerator::raw_str() { return lua.str(); }
void LuaConfigGenerator::lua_append(std::string raw_lua_code) {
  // Simply injects the pre-serialized block into the stream
  this->lua << raw_lua_code;
}

} // namespace telemetry
