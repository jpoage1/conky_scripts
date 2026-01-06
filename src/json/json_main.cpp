
#include "cli_parser.hpp"
#include "configuration_builder.hpp"
#include "context.hpp"
#include "controller.hpp"
#include "json_definitions.hpp"
#include "log.hpp"
#include "lua_parser.hpp"
#include "metrics.hpp"
#include "output_mode_json.hpp"
#include "parsed_config.hpp"
#include "polling.hpp"
#include "runner_context.hpp"
#include "stream_provider.hpp"
#include "telemetry.hpp"
#include "types.hpp"

int json_main(const RunnerContext& context) {
  const ControllerPtr& controller = context.controller;
  do {
    controller->sleep();
    controller->tick();
  } while (controller->is_persistent());

  return 0;
}
