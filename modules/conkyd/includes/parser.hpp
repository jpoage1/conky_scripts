// parser.hpp
#pragma once

#include <string>
#include <vector>
#include <chrono>

#include "waybar_types.h"  // Includes MetricResult

enum RunMode {
    RUN_ONCE,
    PERSISTENT,
};
class ParsedConfig {
    private:
        std::chrono::nanoseconds pooling_interval = std::chrono::milliseconds(500);
    public:
        std::vector<MetricResult> tasks;
        RunMode mode = RUN_ONCE;

        /**
         * @brief A single template setter.
         * It accepts any std::chrono duration and converts it to nanoseconds.
         */
        template <typename DurationType>
        void set_pooling_interval(const DurationType& interval) {
            pooling_interval = std::chrono::duration_cast<std::chrono::nanoseconds>(interval);
        }

        /**
         * @brief A single template getter.
         * It returns the interval cast to whatever type the caller asks for.
         */
        template <typename DurationType>
        DurationType get_pooling_interval() const {
            return std::chrono::duration_cast<DurationType>(pooling_interval);
        }
        RunMode get_run_mode() const {
            return mode;
        }

};
/**
 * @brief Parses command line arguments (argc, argv) to collect metric results.
 * Handles initial argument checks and conversion.
 * @param argc Argument count from main.
 * @param argv Argument vector from main.
 * @return A vector containing the results (MetricResult). Empty if critical
 * errors occur.
 */
ParsedConfig parse_arguments(int argc,
                                          char* argv[]);  // Changed signature
