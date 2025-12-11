// parser.hpp
#pragma once

#include "json_definitions.hpp"
#include "lua_parser.hpp"
#include "metrics.hpp"
#include "pcn.hpp"
#include "runner.hpp"

// The generic signature for ANY output strategy
using OutputPipeline = std::function<void(const std::vector<SystemMetrics>&)>;

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
  std::chrono::nanoseconds polling_interval = std::chrono::milliseconds(500);
  OutputMode _output_mode = JSON;
  RunMode _run_mode = RUN_ONCE;
  OutputPipeline active_pipeline;
  std::chrono::time_point<std::chrono::steady_clock> sleep_until;

 public:
  std::vector<MetricsContext> tasks;

  /**
   * @brief A single template setter.
   * It accepts any std::chrono duration and converts it to nanoseconds.
   */
  template <typename DurationType>
  void set_polling_interval(const DurationType& interval) {
    polling_interval =
        std::chrono::duration_cast<std::chrono::nanoseconds>(interval);
  }

  /**
   * @brief A single template getter.
   * It returns the interval cast to whatever type the caller asks for.
   */
  template <typename DurationType>
  DurationType get_polling_interval() const {
    return std::chrono::duration_cast<DurationType>(polling_interval);
  }

  bool run_mode(RunMode mode) const;
  bool output_mode(OutputMode mode) const;
  RunMode run_mode() const;
  OutputMode get_output_mode() const;
  void set_run_mode(RunMode mode);
  void set_output_mode(OutputMode mode);
  void set_output_mode(std::string mode);
  void set_run_mode(std::string mode);
  void configure_renderer();
  void sleep();

  int initialize(std::vector<SystemMetrics>& tasks,
                 std::chrono::time_point<std::chrono::steady_clock>& timestamp);

  int initialize(std::vector<SystemMetrics>& tasks);
  void done(std::vector<SystemMetrics>& result);
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

// Factory functions: They take settings and return a runnable function
OutputPipeline configure_json_pipeline(const MetricSettings& settings);
OutputPipeline configure_conky_pipeline(const MetricSettings& settings);
