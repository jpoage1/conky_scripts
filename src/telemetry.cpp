
#include "cli_parser.hpp"
#include "config_types.hpp"
#include "configuration_builder.hpp"
#include "context.hpp"
#include "controller.hpp"
#include "json_definitions.hpp"
#include "log.hpp"
#include "lua_parser.hpp"
#include "metrics.hpp"
#include "polling.hpp"
#include "stream_provider.hpp"

int main(int argc, char* argv[]) {
  // Phase 1: Parse Text
  // Phase 1: Parse CLI
  ProgramOptions options = parse_cli(argc, argv);

  // Phase 2: Build Config (Reusable/Testable Path)
  ParsedConfig config = build_config_from_options(options);

  // Phase 3: Execute (The Game/Standalone Path)
  Controller controller;
  controller.initialize(config);

  do {
    controller.sleep();
    controller.tick();
  } while (controller.is_persistent());

  return 0;
}
