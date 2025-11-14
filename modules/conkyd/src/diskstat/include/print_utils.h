#pragma once

#include <tuple>
#include <vector>

#include "types.h"

void print_rows(const std::vector<DeviceInfo>&, const size_t column_count);
void print_column_headers(std::tuple<std::string, std::function<FuncType>>[],
                          size_t);
