#ifndef LUA_GENERATOR_HPP
#define LUA_GENERATOR_HPP

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace telemetry {

struct MetricSettings;
class LuaConfigGenerator {
  using string = std::string;
  std::string name;

public:
  const std::string tab = "  ";

  LuaConfigGenerator(int level = 0) : indentation_level(level) {}
  LuaConfigGenerator(std::string name, int level = 0)
      : name(name), indentation_level(level) {}

  void indent(int diff) {
    indentation_level += diff;
    indent();
  }
  void indent() {
    for (int i = 0; i < indentation_level; i++)
      lua << tab;
  }

  void lua_bool(string name, bool default_value) {
    indent();
    this->lua << name << " = " << default_value << std::endl;
  }
  void lua_uint(string name, unsigned int default_value) {
    indent();
    this->lua << name << " = " << default_value << std::endl;
  }
  void lua_int(string name, int default_value) {
    indent();
    this->lua << name << " = " << default_value << std::endl;
  }
  void lua_double(string name, double default_value) {
    indent();
    this->lua << name << " = " << default_value << std::endl;
  }
  void lua_string(string name, string default_value) {
    indent();
    this->lua << name << " = \"" << default_value << "\"" << std::endl;
  }
  void lua_wrap(std::string text) {
    indent();
    lua << "{" << std::endl;
    lua << text;
    indent();
    lua << "}" << std::endl;
  }
  void lua_vector(const std::vector<string> &values, bool indent_first = true) {
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
  void lua_vector(string name, const std::vector<string> &values) {
    indent();
    lua << name << " = ";
    lua_vector(values);
  }
  void lua_raw_list(const std::vector<string> &entries) {
    std::stringstream ss;
    for (size_t i = 0; i < entries.size(); ++i) {
      ss << entries[i] << (i == entries.size() - 1 ? "" : ",\n");
    }
    lua_wrap(ss.str());
  }
  void lua_raw_list(string name, const std::vector<string> &entries) {
    std::stringstream ss;
    for (size_t i = 0; i < entries.size(); ++i) {
      ss << entries[i] << (i == entries.size() - 1 ? "" : ",\n");
    }
    this->lua << lua_mklist(name, ss.str());
  }
  void lua_list_entry(std::string entry) {
    lua << "\"" << entry << "\"" << ", ";
  }
  void lua_list_raw_entry(std::string entry) { lua << entry << ", "; }
  std::string lua_mklist(string name, string list) {
    indent(1);
    std::stringstream str;
    str << name << " = {" << std::endl;
    str << list;
    indent();
    str << "}" << std::endl;
    return str.str();
  }
  string str() {
    if (name != "")
      return lua_mklist(name, lua.str());
    else {
      lua_wrap(lua.str());
      return lua.str();
    }
  }
  string raw_str() { return lua.str(); }
  void lua_append(std::string raw_lua_code) {
    // Simply injects the pre-serialized block into the stream
    this->lua << raw_lua_code;
  }

private:
  std::stringstream lua;
  int indentation_level;
};

} // namespace telemetry

#endif
