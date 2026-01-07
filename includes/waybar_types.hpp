// waybar_types.hpp
#ifndef WAYBAR_TYPES_HPP
#define WAYBAR_TYPES_HPP

#include "format.hpp"
namespace telemetry {

struct FormattedSize {

  std::string text;
  std::string color;

  std::string formatted(TargetFormat target) const {
    if (target == TargetFormat::WAYBAR) {
      return "<span foreground='#" + color + "'>" + text + "</span>";
    }

    return text;
  }
};

}; // namespace telemetry

#endif
