// parser.cpp
#include "parser.hpp"

#include <iostream>
#include <vector>

#include "waybar_cli_parser.hpp"


ParsedConfig parse_arguments(int argc,
                                          char* argv[]) {  // Changed signature
  ParsedConfig config;
  const char* prog_name = argv[0];  // Get program name

  // --- Moved argc check here ---
  if (argc < 2) {
    std::cerr << "Error: No arguments provided." << std::endl;
    print_usage(prog_name);
    return config;  // Return empty
  }

  // --- Moved vector conversion here ---
  std::vector<std::string> args_in(argv + 1,
                                   argv + argc);  // args_in is argv[1] onwards

  // Reconstruct argv-style vector needed by process_command logic
  std::vector<std::string> args_full;
  args_full.push_back(prog_name);  // argv[0] equivalent
  args_full.insert(args_full.end(), args_in.begin(), args_in.end());

  size_t i = 1;  // Index for args_full (starts at argv[1] equivalent)

  // --- THE REST OF THE PARSING LOOP REMAINS THE SAME ---
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
      config.tasks.push_back(result);
      i++;  // Consume unknown command
    }
    // Backup check to prevent infinite loop if consumed is weirdly negative or
    // causes no change This depends heavily on how process_command signals
    // consumption (return value vs reference update) Assuming process_command
    // updates 'i' by reference correctly, this might be redundant.

  }  // end while

  // Final checks (remain the same)
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
