

#include "parser.hpp"
#include "waybar_formatters.h"
#include "waybar_types.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  std::vector<std::string> args(argv, argv + argc);
  std::vector<MetricsContext> all_results;
  size_t i = 1;  // Use size_t for index

  while (i < args.size()) {
    std::string command = args[i];
    int consumed = 0;

    // --- Handle Implicit Local (First argument only) ---
    if (i == 1 && command.rfind("--", 0) != 0) {
      // Temporarily prepend "--local" to simulate the command structure
      std::vector<std::string> temp_args = {"program_name", "--local"};
      temp_args.insert(temp_args.end(), args.begin() + 1, args.end());
      size_t temp_i = 1;  // Start processing at "--local"
      consumed = process_command(temp_args, temp_i, all_results);
      // Adjust consumption: consumed includes the fake "--local", subtract 1
      consumed = (consumed > 0) ? consumed - 1 : 0;

      // --- Handle Explicit Commands ---
    } else if (command == "--local" || command == "--ssh") {
      consumed = process_command(args, i, all_results);

      // --- Handle Unknown ---
    } else {
      MetricsContext result;
      result.source_name = "Parser";
      result.success = false;
      result.error_message = "Unknown command or flag: " + command;
      all_results.push_back(std::move(result));
      consumed = 1;  // Consume the unknown command
    }

    // Advance index by the number of arguments consumed by the helper
    i += (consumed > 0) ? consumed : 1;  // Ensure progress even on error

  }  // end while loop

  if (all_results.empty()) {
    // Check if any *successful* results were processed.
    bool any_success = false;
    for (const auto& res : all_results) {
      if (res.success) {
        any_success = true;
        break;
      }
    }
    if (!any_success && !all_results.empty()) {
      // Only errors were generated, still output them
    } else if (all_results.empty()) {
      std::cerr << "Error: No valid commands were processed." << std::endl;
      print_usage(argv[0]);
      return 1;
    }
  }
  generate_waybar_output(all_results);
  return 0;
}
