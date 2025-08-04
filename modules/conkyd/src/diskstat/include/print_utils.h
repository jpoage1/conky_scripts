#pragma once

#include "types.h"
#include <vector>
#include <tuple>

void print_rows(std::vector<DeviceInfo>&, size_t column_count);
void print_column_headers(std::tuple<std::string, std::function<FuncType>>[], size_t);

