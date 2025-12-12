// cli_parser.hpp
#pragma once

#include "context.hpp"
#include "json_definitions.hpp"
#include "lua_parser.hpp"
#include "metrics.hpp"
#include "pcn.hpp"
#include "types.hpp"
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
  std::optional<std::string> global_config_file;  // --config
  bool persistent = false;                        // --persistent

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
ParsedConfig parse_arguments(int argc, char* argv[]);

void print_usage(const char* prog_name);

// Factory functions: They take settings and return a runnable function
OutputPipeline configure_json_pipeline(const MetricSettings& settings);
OutputPipeline configure_conky_pipeline(const MetricSettings& settings);

std::set<std::string> parse_interface_list(const std::string& list_str);
ProgramOptions parse_token_stream(int argc, char* argv[]);
ProgramOptions parse_cli(int argc, char* argv[]);
