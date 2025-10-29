#pragma once

#include <filesystem>
#include <set>
#include <string>
#include <vector>

#include "metrics.hpp"
#include "waybar_types.h"

void print_usage(const char* prog_name);

int check_config_file(const std::string& config_file);

std::set<std::string> parse_interface_list(const std::string& list_str);

int process_command(const std::vector<std::string>& args, size_t& current_index,
                    std::vector<MetricResult>& all_results);
