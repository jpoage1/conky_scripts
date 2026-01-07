#ifndef LUA_GENERATOR_HPP
#define LUA_GENERATOR_HPP

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace telemetry {

struct MetricSettings;

class LuaConfigGenerator {
  std::string name;

public:
  using string = std::string;
  const std::string tab = "  ";

  LuaConfigGenerator(unsigned int level = 0);
  LuaConfigGenerator(const std::string &name, unsigned int level = 0);

  void indent(int diff);
  void indent();
  void lua_bool(const string &name, const bool &default_value);
  void lua_uint(const string &name, const unsigned int &default_value);
  void lua_int(const string &name, const int &default_value);
  void lua_double(const string &name, const double &default_value);
  void lua_string(const string &name, const string &default_value);
  void lua_wrap(const std::string &text);
  void lua_vector(const std::vector<string> &values,
                  const bool &indent_first = true);
  void lua_vector(const string &name, const std::vector<string> &values);
  void lua_raw_list(const std::vector<string> &entries);
  void lua_raw_list(const string &name, const std::vector<string> &entries);
  void lua_list_entry(const std::string &entry);
  void lua_list_raw_entry(const std::string &entry);
  const std::string lua_mklist(const string &, const string &);
  string str();
  string raw_str();
  void lua_append(const std::string raw_lua_code);

private:
  std::stringstream lua;
  unsigned int indentation_level;
};

} // namespace telemetry

#endif
