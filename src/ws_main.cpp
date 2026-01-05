
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
#include "ws_server.hpp

int main(int argc, char* argv[]) {
    TelemetryServer ws_server(8080);
    ws_server.start();

    // Register a Socket Factory
    ParsedConfig::register_pipeline(OutputMode::SOCKETS, [&](const MetricSettings& settings) {
        auto serializer = std::make_shared<JsonSerializer>(settings);
        return [&ws_server, serializer](const std::list<SystemMetrics>& results) {
            if (results.empty()) return;
            // Serialize and Broadcast
            ws_server.broadcast(serializer->serialize(results.front()));
        };
    });

    ProgramOptions options = parse_cli(argc, argv);
    ParsedConfig config = build_config_from_options(options);
    config.set_output_mode(OutputMode::SOCKETS);

    Controller controller;
    controller.initialize(config);

    do {
        controller.tick();
        controller.sleep();
    } while (controller.is_persistent());

    return 0;
}
