
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

#include "telemetry.hpp"

int main(int argc, char* argv[]) {
  ControllerPtr controller = initialize(argc, argv);

  do {
    controller->sleep();
    controller->tick();
  } while (controller->is_persistent());

  return 0;
}
