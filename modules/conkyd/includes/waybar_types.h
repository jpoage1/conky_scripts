// waybar_types.hpp
#pragma once

struct TargetFormat;

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
