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
  // 4. Instantiate Commands
  for (const auto& cmd : options.commands) {
    MetricsContext context;

    // --- STEP 1: Load Base Configuration (The "Defaults") ---
    // If a config file path exists, load it first to populate the context.
    // This applies to SETTINGS, LOCAL (if file provided), and SSH (if file
    // provided).
    if (!cmd.config_path.empty()) {
      // We use load_lua_settings for everything initially to get the base
      // 'settings' table Note: specific LOCAL/SSH config loading might differ
      // slightly based on your legacy logic, but typically you load the Lua
      // "settings" table to get the baseline.

      // Check if file exists before trying to load to avoid confusing Lua
      // errors if the user just passed a raw string meant for something else.
      if (std::filesystem::exists(cmd.config_path)) {
        context = load_lua_settings(cmd.config_path);
      } else {
        // If it's not a file, strict modes might want to error,
        // but for LOCAL context.device_file might just be the file string
        // itself if used differently. For now, we assume if it's a file, we
        // load settings.
        context.device_file = cmd.config_path;
      }
    }

    // --- STEP 2: Apply Command-Type Specific Defaults ---
    if (cmd.type == CommandType::LOCAL) {
      context.provider = DataStreamProviders::LocalDataStream;
      context.source_name = "Local";
      // Ensure device_file is set if load_lua_settings didn't set it/wasn't
      // used
      if (context.device_file.empty()) context.device_file = cmd.config_path;
    } else if (cmd.type == CommandType::SSH) {
      context.provider = DataStreamProviders::ProcDataStream;
      context.source_name = "Default SSH";  // Default, overridden below
    }

    // --- STEP 3: Merge CLI Arguments (The "Overrides") ---

    // A. Interfaces: MERGE (Union of Lua set + CLI set)
    if (!cmd.interfaces.empty()) {
      // This adds CLI interfaces to existing Lua interfaces.
      // If you want CLI to REPLACE Lua completely, use '=' instead of 'insert'.
      context.interfaces.insert(cmd.interfaces.begin(), cmd.interfaces.end());
    }

    // B. SSH Credentials: OVERRIDE
    if (cmd.type == CommandType::SSH) {
      if (!cmd.host.empty() && !cmd.user.empty()) {
        // CLI explicit host/user overrides anything found in Lua
        context.host = cmd.host;
        context.user = cmd.user;
        context.source_name = cmd.user + "@" + cmd.host;
      }
      // If CLI didn't provide host/user, we stick with what load_lua_settings
      // found (if any)
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
