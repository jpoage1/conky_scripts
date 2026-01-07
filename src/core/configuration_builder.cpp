// configuration_builder.hpp / .cpp
#include "configuration_builder.hpp"

#include "cli_parser.hpp"
#include "configuration_builder.hpp"
#include "context.hpp"
#include "controller.hpp"
#include "json_definitions.hpp"
#include "log.hpp"
#include "lua_parser.hpp"
#include "metrics.hpp"
#include "parsed_config.hpp"
#include "polling.hpp"
#include "stream_provider.hpp"

namespace telemetry {

/**
 * Path 2: CLI-to-Config Translation
 * Encapsulates the logic of converting CLI options into a runnable
 * configuration.
 */
ParsedConfig build_config_from_options(const ProgramOptions &options) {
  ParsedConfig config;

  if (options.global_config_file.has_value()) {
    // Path 1: Pure Lua Path
    return load_lua_config(options.global_config_file.value());
  }

  // Path 2: CLI Command Path
  if (options.persistent) {
    config.set_run_mode(RunMode::PERSISTENT);
  }

  for (const auto &cmd : options.commands) {
    config.tasks.push_back(create_context_from_command(cmd));
  }

  return config;
}

/**
 * Helper to isolate specific command mapping.
 * This is now individually testable.
 */
MetricsContext create_context_from_command(const CommandRequest &cmd) {
  MetricsContext context;

  switch (cmd.type) {
  case CommandType::LOCAL:
    context.provider = DataStreamProviders::LocalDataStream;
    context.device_file = cmd.config_path;
    break;

  case CommandType::SSH:
    context.device_file = cmd.config_path;
    context.interfaces = cmd.interfaces;
    if (!cmd.host.empty() && !cmd.user.empty()) {
      context.source_name = cmd.user + "@" + cmd.host;
      context.provider = DataStreamProviders::ProcDataStream;
      context.host = cmd.host;
      context.user = cmd.user;
    } else {
      context.source_name = "Default SSH";
      context.provider = DataStreamProviders::ProcDataStream;
    }
    break;

  case CommandType::SOCKETS:
    std::cerr << "Function not implemented" << std::endl;
    break;

  case CommandType::SETTINGS:
    context = load_lua_settings(cmd.config_path);
    break;
  }
  return context;
}
}; // namespace telemetry
