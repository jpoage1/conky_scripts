// conky_output.hpp
#pragma once
#include "format.hpp"
#include "pcn.hpp"

struct DeviceInfo;
struct CombinedMetrics;
struct SystemMetrics;

const int CHAR_WIDTH_PX = 10;     // approximate font width
const int DEFAULT_COL_WIDTH = 8;  // fallback width in characters

void print_metrics(const std::vector<DeviceInfo>& devices);
void print_metrics(const CombinedMetrics& metrics);
void print_metrics(const SystemMetrics& metrics);

void print_rows(const std::vector<DeviceInfo>&, const size_t column_count);
void print_column_headers(std::tuple<std::string, std::function<FuncType>>[],
                          size_t);
