#pragma once

#include "colors.h"
#include "format.hpp"

ColoredString conky_color(const std::string&, const std::string&);

ColoredString format_size(uint64_t bytes);
