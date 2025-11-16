// parser.hpp
#pragma once

#include "json_definitions.hpp"
#include "metrics.hpp"
#include "pcn.hpp"
#include "runner.hpp"

enum RunMode {
  RUN_ONCE,
  PERSISTENT,
};
enum OutputMode {
  CONKY,
  JSON,
};
class ParsedConfig {
 private:
  std::chrono::nanoseconds pooling_interval = std::chrono::milliseconds(500);
  OutputMode _output_mode = JSON;
  RunMode _run_mode = RUN_ONCE;

 public:
  std::vector<MetricsContext> tasks;

  /**
   * @brief A single template setter.
   * It accepts any std::chrono duration and converts it to nanoseconds.
   */
  template <typename DurationType>
  void set_pooling_interval(const DurationType& interval) {
    pooling_interval =
        std::chrono::duration_cast<std::chrono::nanoseconds>(interval);
  }

  /**
   * @brief A single template getter.
   * It returns the interval cast to whatever type the caller asks for.
   */
  template <typename DurationType>
  DurationType get_pooling_interval() const {
    return std::chrono::duration_cast<DurationType>(pooling_interval);
  }

  bool run_mode(RunMode mode) const;
  bool output_mode(OutputMode mode) const;
  RunMode run_mode() const;
  OutputMode get_output_mode() const;
  void set_run_mode(RunMode mode);
  void set_output_mode(OutputMode mode);
  void done();
};
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
