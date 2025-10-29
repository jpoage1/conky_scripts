// parser.hpp
#pragma once

#include <string>
#include <vector>

#include "waybar_types.h"  // Includes MetricResult

/**
 * @brief Parses command line arguments (argc, argv) to collect metric results.
 * Handles initial argument checks and conversion.
 * @param argc Argument count from main.
 * @param argv Argument vector from main.
 * @return A vector containing the results (MetricResult). Empty if critical
 * errors occur.
 */
std::vector<MetricResult> parse_arguments(int argc,
                                          char* argv[]);  // Changed signature
