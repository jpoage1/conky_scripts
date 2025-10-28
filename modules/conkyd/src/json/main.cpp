#include "nlohmann/json.hpp"

#include <iostream>
#include <vector>

#include "json_definitions.hpp"  // JSON serialization macros
#include "parser.hpp"            // Shared parser (handles argc check now)
#include "waybar_types.h"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
  // 1. Call the parser (handles initial checks and processing)
  std::vector<MetricResult> all_results = parse_arguments(argc, argv);

  // 2. If parser returned potentially valid (even if error) results, generate
  // output
  if (!(argc < 2 && all_results.empty())) {
    json output_json = all_results;
    std::cout << output_json.dump() << std::endl;
  }

  // Determine exit code
  bool any_success = false;
  for (const auto& res : all_results) {
    if (res.success) {
      any_success = true;
      break;
    }
  }
  return any_success ? 0 : 1;
}
