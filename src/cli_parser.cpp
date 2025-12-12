// cli_parser.cpp
#include "cli_parser.hpp"

// #include "CLI/CLI.hpp"
#include <iostream>
#include <sstream>
#include <vector>

#include "cli_parser.hpp"
#include "config_types.hpp"
#include "context.hpp"
#include "data_local.hpp"
#include "data_ssh.hpp"
#include "log.hpp"
#include "lua_parser.hpp"
#include "polling.hpp"

// Helper: Parse "eth0,wlan0" into a set
std::set<std::string> parse_interface_list(const std::string& list_str) {
  std::set<std::string> interfaces;
  std::stringstream ss(list_str);
  std::string item;
  while (std::getline(ss, item, ',')) {
    if (!item.empty()) interfaces.insert(item);
  }
  return interfaces;
}

// ---------------------------------------------------------
// PHASE 1: PARSING (Text -> Options)
// ---------------------------------------------------------
ProgramOptions parse_cli(int argc, char* argv[]) {
  ProgramOptions options;

  if (argc < 2) {
    std::cerr << "Error: No arguments provided.\n";
    print_usage(argv[0]);
    return options;
  }

  std::vector<std::string> args(argv + 1, argv + argc);

  for (size_t i = 0; i < args.size(); ++i) {
    const std::string& arg = args[i];

    // --- 1. Global Flags ---
    if (arg == "--config") {
      if (i + 1 < args.size()) {
        options.global_config_file = args[++i];
        return options;  // Exclusive Mode: Stop parsing immediately
      } else {
        std::cerr << "Error: --config requires a filename.\n";
      }
    } else if (arg == "--persistent") {
      options.persistent = true;
    }

    // --- 2. Primary Commands ---
    else if (arg == "--local") {
      if (i + 1 < args.size()) {
        options.commands.push_back({CommandType::LOCAL, args[++i], {}, "", ""});
      } else {
        std::cerr << "Error: --local requires a config file.\n";
      }
    } else if (arg == "--ssh") {
      if (i + 1 < args.size()) {
        CommandRequest req;
        req.type = CommandType::SSH;
        req.config_path = args[++i];  // Consume config file

        // Check for optional Host/User arguments
        // We peek ahead 1 and 2 spots. If they exist and don't start with '-',
        // consume them.
        if (i + 2 < args.size()) {
          std::string potential_host = args[i + 1];
          std::string potential_user = args[i + 2];

          if (potential_host[0] != '-' && potential_user[0] != '-') {
            req.host = potential_host;
            req.user = potential_user;
            i += 2;  // Consume host and user
          }
        }

        // Initialize empty interfaces (will be filled if --interfaces follows)
        req.interfaces = {};
        options.commands.push_back(req);
      } else {
        std::cerr << "Error: --ssh requires a config file.\n";
      }
    } else if (arg == "--settings") {
      if (i + 1 < args.size()) {
        options.commands.push_back(
            {CommandType::SETTINGS, args[++i], {}, "", ""});
      } else {
        std::cerr << "Error: --settings requires a config file.\n";
      }
    }

    // --- 3. Context Modifiers (--interfaces) ---
    // Applies to the MOST RECENTLY added command
    else if (arg == "--interfaces") {
      if (options.commands.empty()) {
        std::cerr << "Warning: --interfaces ignored (no preceding command).\n";
        if (i + 1 < args.size()) i++;  // Consume value anyway to stay in sync
      } else if (i + 1 < args.size()) {
        std::string list_str = args[++i];
        options.commands.back().interfaces = parse_interface_list(list_str);
      } else {
        std::cerr << "Error: --interfaces requires a list (e.g. eth0,wlan0).\n";
      }
    }

    // --- 4. Implicit Local (Positional Arg) ---
    // If it's not a flag (doesn't start with -), treat as local config
    else if (arg[0] != '-') {
      options.commands.push_back({CommandType::LOCAL, arg, {}, "", ""});
    } else {
      // Deprecated/Ignored flags (like --ssh)
      std::cerr << "Warning: Unknown or deprecated argument ignored: " << arg
                << "\n";
    }
  }

  return options;
}

// ---------------------------------------------------------
// PHASE 2: BUILDING (Options -> Runtime Config)
// ---------------------------------------------------------
ParsedConfig parse_arguments(int argc, char* argv[]) {
  // 1. Get Clean Options
  ProgramOptions options = parse_cli(argc, argv);

  ParsedConfig config;

  // 2. Handle Exclusive Mode
  if (options.global_config_file.has_value()) {
    std::cerr << "Loading global config: " << options.global_config_file.value()
              << std::endl;
    config = load_lua_config(options.global_config_file.value());
    config.set_filename(options.global_config_file.value());
    return config;
  }

  // 3. Handle Flags
  if (options.persistent) {
    config.set_run_mode(RunMode::PERSISTENT);
  }

  // 4. Instantiate Commands
  for (const auto& cmd : options.commands) {
    MetricsContext context;

    if (cmd.type == CommandType::LOCAL) {
      context.provider = DataStreamProviders::LocalDataStream;
      context.device_file = cmd.config_path;
      context.interfaces = cmd.interfaces;  // Apply filtered interfaces
      context.source_name = "Local";
    } else if (cmd.type == CommandType::SETTINGS) {
      context = load_lua_settings(cmd.config_path);
      // Settings might have interfaces defined in Lua,
      // but CLI overrides usually take precedence if you wanted to merge them.
      if (!cmd.interfaces.empty()) {
        context.interfaces = cmd.interfaces;
      }
    }

    config.tasks.push_back(std::move(context));
  }

  return config;
}

void print_usage(const char* prog_name) {
  std::cerr
      << "Usage: " << prog_name << " [options...]\n\n"
      << "Generates metrics based on one or more commands.\n"
      << "If the first argument is a file path, it defaults to --local.\n\n"
      << "Commands:\n"
      << "  <config_file>       (As first argument) Generate local metrics.\n"
      << "  --local <config_file>\n"
      << "                      Generate local metrics.\n"
      << "  --ssh <config_file>\n"
      << "                      Generate metrics from default SSH host.\n"
      << "  --ssh <config_file> <host> <user>\n"
      << "                      Generate metrics from specific SSH host.\n\n"
      << "Example:\n"
      << "  " << prog_name
      << " /path/local.conf --ssh /path/ssh.conf my-server conky\n";
}
