#pragma once

#include <unordered_map>
const int CHAR_WIDTH_PX = 10;     // approximate font width
const int DEFAULT_COL_WIDTH = 8;  // fallback width in characters
std::unordered_map<size_t, int> column_widths = {
    {0, 20},  // column 0 has width 20
    {1, 20},  // column 1 has width 20
              // columns 2+ will use default
};
