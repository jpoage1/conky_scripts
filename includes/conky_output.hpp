// conky_output.hpp
#ifndef CONKY_OUTPUT_HPP
#define CONKY_OUTPUT_HPP

#include "pcn.hpp"

namespace telemetry {

struct DeviceInfo;
struct SystemMetrics;
struct ColoredString;

using FuncType = ColoredString(const DeviceInfo &);

const int CHAR_WIDTH_PX = 10;    // approximate font width
const int DEFAULT_COL_WIDTH = 8; // fallback width in characters

void print_metrics(const std::vector<DeviceInfo> &devices);
void print_metrics(const SystemMetrics &metrics);

void print_rows(const std::vector<DeviceInfo> &, const size_t column_count);
void print_column_headers(std::tuple<std::string, std::function<FuncType>>[],
                          size_t);
}; // namespace telemetry
#endif
