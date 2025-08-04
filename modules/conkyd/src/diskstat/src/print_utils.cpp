#include "conky_format.h"
#include "constants.h"
#include "print_utils.h"
#include <iomanip>
#include <iostream>

void pad_str(const std::string &str, int width) {
  std::cout << std::left << std::setw(width) << str;
}

void pad_str(const ColoredString &cstr, int width) {
  std::ostringstream ss;
  ss << std::left << std::setw(width) << cstr.text;
  std::cout << "${color " << cstr.color << "}" << ss.str() << "${color}";
}

void print_column_headers(
    std::tuple<std::string, std::function<FuncType>> columns[], size_t count) {
  std::cout << "${color " << paleblue << "}";
  for (size_t i = 0; i < count; ++i) {
    int width = i == 0 ? COL_WIDTH_1 : COL_WIDTH;
    pad_str(std::get<0>(columns[i]), width);
  }
  std::cout << "${color}" << std::endl;
}

void print_rows(std::vector<DeviceInfo> &devices, size_t column_count) {
  extern std::tuple<std::string, std::function<FuncType>> columns[];
  for (const auto &device : devices) {
    for (size_t i = 0; i < column_count; ++i) {
      int width = i == 0 ? COL_WIDTH_1 : COL_WIDTH;
      pad_str(std::get<1>(columns[i])(device), width);
    }
    std::cout << std::endl;
  }
}
