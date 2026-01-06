
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

#include "telemetry.hpp"

ControllerPtr initialize(int argc, char* argv[]) {
    ProgramOptions options = parse_cli(argc, argv);
    ParsedConfig config = build_config_from_options(options);
    ControllerPtr controller = std::make_unique<Controller>();
    controller->initialize(config);
    return controller;
}
