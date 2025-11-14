// parser.cpp
#include "parser.hpp"

#include <iostream>
#include <vector>
#include <set>

#include "waybar_types.h"


ParsedConfig parse_arguments(int argc,
                                          char* argv[]) {
  ParsedConfig config;
  const char* prog_name = argv[0];  // Get program name

  if (argc < 2) {
    std::cerr << "Error: No arguments provided." << std::endl;
    print_usage(prog_name);
    return config;  // Return empty
  }

  std::vector<std::string> args_in(argv + 1,
                                   argv + argc);  // args_in is argv[1] onwards

  std::vector<std::string> args_full;
  args_full.push_back(prog_name);  // argv[0] equivalent
  args_full.insert(args_full.end(), args_in.begin(), args_in.end());

  size_t i = 1;  // Index for args_full (starts at argv[1] equivalent)

  while (i < args_full.size()) {
    std::string command = args_full[i];
    int consumed = 0;

    // Handle Implicit Local
    if (i == 1 && command.rfind("--", 0) != 0) {
      std::vector<std::string> temp_args = {prog_name, "--local"};
      temp_args.insert(temp_args.end(), args_in.begin(), args_in.end());
      size_t temp_i = 1;
      consumed = process_command(temp_args, temp_i, config.tasks);
      consumed = (consumed > 0) ? consumed - 1 : 0;
      i += (consumed > 0) ? consumed : 1;
    }
    // Handle Persistent Flag
    else if (command == "--persistent") {
        config.mode = PERSISTENT;
        i++; // Consume this flag and continue parsing
    }
    // Handle Explicit Commands
    else if (command == "--local" || command == "--ssh") {
      consumed = process_command(args_full, i, config.tasks);
      // Ensure loop advances if process_command fails/returns 0 or doesn't
      // update 'i'
      if (consumed == 0) {
        i++;  // Prevent infinite loop
      }
      // If process_command updates 'i' by reference, this 'else' block isn't
      // needed else {
      //    i += consumed;
      // }
    }
    // Handle Unknown
    else {
      MetricResult result;
      result.source_name = "Parser";
      result.success = false;
      result.error_message = "Unknown command or flag: " + command;
    //   config.tasks.push_back(result);
      config.tasks.push_back(std::move(result));
      i++;  // Consume unknown command
    }
  }  // end while

  // Final checks
  if (config.tasks.empty() && config.mode == RUN_ONCE) {
    std::cerr << "Error: No valid commands resulted in metrics." << std::endl;
  }
  else {
    // bool any_success = false;
    // for (const auto& res : all_results) {
    //   if (res.success) {
    //     any_success = true;
    //     break;
    //   }
    // }
    // if (!any_success) {
    //   std::cerr << "Warning: All processed commands resulted in errors."
    //             << std::endl;
    // }
  }

  return config;
}


int check_config_file(const std::string& config_file) {
  if (!std::filesystem::exists(config_file)) {
    std::cerr << "Warning: Config file not found, skipping: " << config_file
              << std::endl;
    return 1;  // Return 1 to indicate an error
  }
  return 0;  // Success
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

std::set<std::string> parse_interface_list(const std::string& list_str) {
  std::set<std::string> interfaces;
  std::stringstream ss(list_str);
  std::string interface_name;
  while (std::getline(ss, interface_name, ',')) {
    if (!interface_name.empty()) {
      interfaces.insert(interface_name);
    }
  }
  return interfaces;
}
int process_command(const std::vector<std::string>& args, size_t& current_index,
                    std::vector<MetricResult>& tasks) {
  MetricResult result;
  std::string command = args[current_index];
  size_t initial_index = current_index;

  // --- 1. Parse Config File ---
  if (current_index + 1 >= args.size()) {
    result.success = false;
    result.error_message = command + " requires a <config_file> argument.";
    result.source_name =
        (command == "--local") ? "Local (Error)" : "SSH (Error)";
    tasks.push_back(std::move(result));
    return 1;  // Consume only the command itself
  }
  std::string config_file = args[current_index + 1];
  current_index += 2;  // Tentatively consume command + config

  // --- 2. Check Config File ---
  if (check_config_file(config_file) != 0) {
    result.success = false;
    result.error_message = "Config file not found: " + config_file;
    result.source_name =
        (command == "--local") ? "Local (Error)" : "SSH (Error)";
    tasks.push_back(std::move(result));
    return current_index - initial_index;  // Return consumed args
  } else {
    result.device_file = config_file;
  }

  // --- 3. Call Appropriate Get Function ---
  bool success = false;
  if (command == "--local") {
    result.source_name = "Local";
    result.set_callback(get_local_metrics);
    if (!success) result.error_message = "Failed to get local metrics.";

  } else if (command == "--ssh") {
    // Check for specific host/user
    if (current_index + 1 < args.size() &&
        args[current_index].rfind("--", 0) != 0 &&
        args[current_index + 1].rfind("--", 0) != 0) {
      std::string host = args[current_index];
      std::string user = args[current_index + 1];
      result.source_name = user + "@" + host;
    //       (get_server_metrics(config_file, result.metrics, host, user) == 0);
      result.set_callback(
            [host, user](const std::string& cfg, CombinedMetrics& m) {
                // The lambda's body calls the REAL 4-argument function
                return get_server_metrics(cfg, m, host, user);
            }
        );

       success = true;
      if (!success) result.error_message = "Failed to connect.";
      current_index += 2;  // Consume host + user
    } else {
      // Default SSH host
      result.source_name = "Default SSH";
      result.set_callback(
            static_cast<int(*)(const std::string&, CombinedMetrics&)>(get_server_metrics)
        );
      if (!success) result.error_message = "Failed to connect.";
    }
  }
  // --- 4. Check for --interfaces ---
  if (current_index < args.size() && args[current_index] == "--interfaces") {
    if (current_index + 1 < args.size()) {
      result.specific_interfaces =
          parse_interface_list(args[current_index + 1]);
      current_index += 2;  // Consume --interfaces and its value
    } else {
      // Handle error: --interfaces flag without a value
      // You might want to add an error to result.error_message
      std::cerr << "Warning: --interfaces flag requires a comma-separated list."
                << std::endl;
      current_index += 1;  // Consume only the flag to avoid infinite loop
    }
  }

  result.success = success;
  tasks.push_back(std::move(result));
  return current_index - initial_index;  // Return total consumed args
}
