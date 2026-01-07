// src/core/window_settings.cpp

#include "window_settings.hpp"
#include "lua_generator.hpp"
#include <sol/sol.hpp>
#include <string>

namespace telemetry {

// using Value = ScrollState::Value;

// Window Config
ScrollState::ScrollState(ScrollState::Value v) : m_val(v) {}
bool ScrollState::on() const { return m_val != Value::OFF; }
bool ScrollState::off() const { return m_val != Value::ON; }
bool ScrollState::is_auto() const { return m_val == Value::AUTO; }
std::string ScrollState::get_str(const Value &v) {
  switch (v) {
  case ScrollState::Value::ON:
    return "on";
  case ScrollState::Value::OFF:
    return "off";
  case ScrollState::Value::AUTO:
    return "auto";
  default:
    return "unknown";
  }
} // End get_str()
const std::string ScrollState::get_str() const {
  const std::string state = ScrollState::get_str(m_val);
  return state;
}
void ScrollState::set(std::string val) {
  if (val == "on") {
    m_val = ScrollState::Value::ON;
  } else if (val == "off") {
    m_val = ScrollState::Value::OFF;
  } else if (val == "auto") {
    m_val = ScrollState::Value::AUTO;
  } else {
    m_val = ScrollState::Value::OFF; // Fallback for unknown input
  }
} // End set()

std::string LuaScrollState::serialize(std::string name,
                                      unsigned int indentation_level) {
  LuaConfigGenerator gen(indentation_level);
  gen.lua_string(name, get_str());
  return gen.raw_str();
} // End ScrollState::serialize(), redundant and useless

void LuaScrollState::deserialize(sol::object lua_val) {
  if (lua_val.is<std::string>()) {
    set(lua_val.as<std::string>());
  }
}

std::string
LuaScrollDirections::serialize(unsigned int indentation_level) const {
  LuaConfigGenerator gen("scroll", indentation_level);
  gen.lua_string("horizontal", horizontal.get_str());
  gen.lua_string("vertical", vertical.get_str());
  return gen.str();
} // End ScrollDirections::serialize()

void LuaScrollDirections::deserialize(sol::table lua_table) {
  if (!lua_table.valid())
    return;

  LuaScrollState h, v;
  h.deserialize(lua_table["horizontal"]);
  v.deserialize(lua_table["vertical"]);

  horizontal = static_cast<ScrollState>(h);
  vertical = static_cast<ScrollState>(v);
}

std::string LuaWindowConfig::serialize(unsigned int indentation_level) const {
  LuaConfigGenerator gen("window", indentation_level);

  // Primitive types
  gen.lua_string("type", type);
  gen.lua_string("stacking", stacking);
  gen.lua_bool("wmIgnore", wmIgnore);
  gen.lua_int("x", x);
  gen.lua_int("y", y);
  gen.lua_int("width", width);
  gen.lua_int("height", height);
  gen.lua_bool("visible", visible);
  gen.lua_bool("resizable", resizable);

  // Nested ScrollDirections serialization
  // This assumes your LuaConfigGenerator::str() or a raw append method
  // integrates the sub-table string directly into the stream.
  return gen.str();
} // End WindowConfig::serialize()

void LuaWindowConfig::deserialize(sol::table window) {
  if (!window.valid())
    return;

  type = window.get_or("type", std::string("normal"));
  stacking = window.get_or("stacking", std::string("bottom"));
  wmIgnore = window.get_or("wmIgnore", true);
  x = window.get_or("x", 0);
  y = window.get_or("y", 0);
  width = window.get_or("width", 800);
  height = window.get_or("height", 600);
  visible = window.get_or("visible", true);
  resizable = window.get_or("resizable", true);

  if (window["scroll"].valid()) {
    LuaScrollDirections lsd;
    lsd.deserialize(window["scroll"]);
    scroll = static_cast<ScrollDirections>(lsd);
  }
}

}; // namespace telemetry
