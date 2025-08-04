#pragma once

#include "colors.h"
#include "types.h"

ColoredString conky_color(const std::string &text,
                          const std::string &color = lightgrey) {
  return ColoredString{text, color};
}
