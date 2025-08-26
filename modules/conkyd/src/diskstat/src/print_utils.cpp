#include "print_utils.h"

#include <iomanip>
#include <iostream>

#include "conky_format.h"
#include "constants.h"

void pad_str(const std::string &str) { std::cout << str; }

void pad_str(const ColoredString &cstr) {
  std::cout << "${color " << cstr.color << "}" << cstr.text << "${color}";
}
void print_column_headers(
    std::tuple<std::string, std::function<FuncType>> columns[], size_t count) {
  std::cout << "${color " << paleblue << "}";
  int xpos = 0;

  for (size_t i = 0; i < count; ++i) {
    std::cout << "${goto " << xpos << "}";
    pad_str(std::get<0>(columns[i]));

    int width = DEFAULT_COL_WIDTH;
    if (column_widths.count(i)) width = column_widths[i];

    xpos += width * CHAR_WIDTH_PX;
  }

  std::cout << "${color}" << std::endl;
}

void print_rows(std::vector<DeviceInfo> &devices, const size_t column_count) {
  extern std::tuple<std::string, std::function<FuncType>> columns[];

  for (const auto &device : devices) {
    int xpos = 0;
    for (size_t i = 0; i < column_count; ++i) {
      std::cout << "${goto " << xpos << "}";
      pad_str(std::get<1>(columns[i])(device));

      int width = DEFAULT_COL_WIDTH;
      if (column_widths.count(i)) width = column_widths[i];

      xpos += width * CHAR_WIDTH_PX;
    }
    std::cout << std::endl;
  }
}
