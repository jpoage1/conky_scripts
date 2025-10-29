// waybar_formatters.hpp
#pragma once

#include <vector>

#include "waybar_types.h"

FormattedSize format_size_rate(double bytes_per_sec);
void generate_waybar_output(const std::vector<MetricResult>& all_results);
