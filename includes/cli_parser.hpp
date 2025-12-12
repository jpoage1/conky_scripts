// cli_parser.hpp
#pragma once

#include "context.hpp"
#include "json_definitions.hpp"
#include "lua_parser.hpp"
#include "metrics.hpp"
#include "pcn.hpp"
#include "types.hpp"

/**
 * @brief Parses command line arguments (argc, argv) to collect metric results.
 * Handles initial argument checks and conversion.
 * @param argc Argument count from main.
 * @param argv Argument vector from main.
 * @return A vector containing the results (MetricsContext). Empty if critical
 * errors occur.
 */
ParsedConfig parse_arguments(int argc, char* argv[]);

void print_usage(const char* prog_name);

int check_config_file(const std::string& config_file);

std::set<std::string> parse_interface_list(const std::string& list_str);

int process_command(const std::vector<std::string>& args, size_t& current_index,
                    std::vector<MetricsContext>& all_results);

// Factory functions: They take settings and return a runnable function
OutputPipeline configure_json_pipeline(const MetricSettings& settings);
OutputPipeline configure_conky_pipeline(const MetricSettings& settings);
