#ifndef WINDOW_SETTINGS_HPP
#define WINDOW_SETTINGS_HPP

#include "pcn.hpp"
#include <string>

namespace telemetry {
class ScrollState {
  enum Value { ON, OFF, AUTO };

public:
  ScrollState(Value v = OFF);
  operator Value() const;
  bool on() const;
  bool off() const;
  bool is_auto() const;
  static std::string get_str(const Value &v);
  const std::string get_str() const;
  void set(std::string val);

private:
  Value m_val;
}; // End ScrollState class

struct ScrollDirections {
public:
  ScrollState horizontal;
  ScrollState vertical;
}; // End ScrollDirections

// Window Config
struct WindowConfig {
  enum WindowTypes { DESKTOP, DOCK, NORMAL };
  enum StackingTypes { FOREGROUND, BACKGROUND, FLOATING };

  ScrollDirections scroll;
  std::string type = "normal";
  std::string stacking = "bottom";
  bool wmIgnore = true;
  int x = 0, y = 0, width = 800, height = 600;
  bool visible = true;
  bool resizable = true;
}; // End WindowConfig struct

class LuaScrollState : public ScrollState {
public:
  std::string serialize(std::string name, unsigned int indentation_level = 0);

  void deserialize(sol::object lua_val);
}; // End ScrollState class
// ScrollDirection

struct LuaScrollDirections : public ScrollDirections {
  std::string serialize(unsigned int indentation_level = 0) const;

  void deserialize(sol::table lua_table);
}; // End ScrollDirections
// Window Config

struct LuaWindowConfig : public WindowConfig {
  std::string serialize(unsigned int indentation_level = 0) const;
  void deserialize(sol::table window);
}; // End WindowConfig struct

}; // namespace telemetry

#endif
