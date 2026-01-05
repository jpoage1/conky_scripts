#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

// Existing headers
#include "controller.hpp"
#include "configuration_builder.hpp"
#include "cli_parser.hpp"
#include "json_serializer.hpp"
#include "config_types.hpp"
#include "metrics.hpp"

// THE MISSING LINK
#include "system_metrics_proxy.hpp" 

// Define the factory logic
PipelineFactory widget_factory(SystemMetricsProxy* proxy) {
    return [proxy](const MetricSettings& settings) -> OutputPipeline {
        auto serializer = std::make_shared<JsonSerializer>(settings);
        return [serializer, proxy](const std::list<SystemMetrics>& results) {
            if (results.empty() || !proxy) return;
            // Push the serialized front-most metric task to the UI
            proxy->updateData(serializer->serialize(results.front()));
        };
    };
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    SystemMetricsProxy proxy;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("backend", &proxy);

    // Register before building config so configure_renderer() finds it
    ParsedConfig::register_pipeline(OutputMode::WIDGETS, widget_factory(&proxy));

    ProgramOptions options = parse_cli(argc, argv);
    ParsedConfig config = build_config_from_options(options);
    config.set_output_mode(OutputMode::WIDGETS);
    
    auto controller = std::make_unique<Controller>();
    controller->initialize(config);

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        controller->tick(); 
        std::cerr << "Test";
    });
    timer.start(1000);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty()) return -1;

    return app.exec();
}
