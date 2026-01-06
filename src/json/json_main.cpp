
#include "cli_parser.hpp"
#include "parsed_config.hpp"
#include "configuration_builder.hpp"
#include "context.hpp"
#include "controller.hpp"
#include "json_definitions.hpp"
#include "log.hpp"
#include "lua_parser.hpp"
#include "metrics.hpp"
#include "polling.hpp"
#include "stream_provider.hpp"
#include "output_mode_json.hpp"
#include "types.hpp"
#include "runner_context.hpp"

#include "telemetry.hpp"


void register_json_pipeline() {
    PipelineEntry pipeline{"json", configure_json_pipeline, json_main, nullptr};
    ParsedConfig::register_pipeline(pipeline);
}

int json_main(const RunnerContext &context) {
  ControllerPtr& controller = context.controller;
  do {
    controller->sleep();
    controller->tick();
  } while (controller->is_persistent());

  return 0;
}
