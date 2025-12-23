// conky_format.hpp
#ifndef CONKY_FORMAT_HPP
#define CONKY_FORMAT_HPP

#include <cstdint>
#include <string>

struct ColoredString;

ColoredString conky_color(const std::string&, const std::string&);

ColoredString format_size(uint64_t bytes);
#endif
