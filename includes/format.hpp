#ifndef FORMAT_HPP
#define FORMAT_HPP

#include "pcn.hpp"

// Enum to define the target output format
enum class TargetFormat { CONKY, WAYBAR };

struct ColoredString {
  std::string text;
  std::string color;

  // Constructor
  ColoredString(const std::string& t, const std::string& c)
      : text(t), color(c) {}

  // Modified method to accept a target format
  std::string formatted(TargetFormat target) const {
    switch (target) {
      case TargetFormat::WAYBAR:
        return "<span foreground='#" + color + "'>" + text + "</span>";

      case TargetFormat::CONKY:
      default:
        return "${color " + color + "}" + text + "${color}";
    }
  }
};
#endif
