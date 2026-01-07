// cli_parser.hpp
#ifndef CLI_PARSER_HPP
#define CLI_PARSER_HPP

#include "pcn.hpp"
namespace telemetry {
struct MetricSettings;
struct ProgramOptions;

class SystemMetrics;
class ParsedConfig;

using OutputPipeline =
    std::function<void(const std::__cxx11::list<SystemMetrics> &)>;

enum class CommandType { LOCAL, SETTINGS, SSH, SOCKETS };

struct CommandRequest {
  CommandType type;
  std::string config_path;
  std::set<std::string> interfaces;

  std::string host;
  std::string user;
};

struct ProgramOptions {
  // Global Flags
  std::optional<std::string> global_config_file; // --config
  bool persistent = false;                       // --persistent

  // The list of tasks to execute
  std::vector<CommandRequest> commands;
};
/**
 * @brief Parses command line arguments (argc, argv) to collect metric results.
 * Handles initial argument checks and conversion.
 * @param argc Argument count from main.
 * @param argv Argument vector from main.
 * @return A vector containing the results (MetricsContext). Empty if critical
 * errors occur.
 */
ParsedConfig parse_arguments(int argc, char *argv[]);

void print_usage(const char *prog_name);

std::set<std::string> parse_interface_list(const std::string &list_str);
ProgramOptions parse_token_stream(int argc, char *argv[]);
ProgramOptions parse_cli(int argc, char *argv[]);
}; // namespace telemetry
#endif
