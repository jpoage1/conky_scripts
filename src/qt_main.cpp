#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include "controller.hpp"
#include "configuration_builder.hpp"
#include "cli_parser.hpp"
#include "system_metrics_proxy.hpp"
#include "json_serializer.hpp"
#include "types.hpp"
#include "config_types.hpp"

PipelineFactory widget_factory(SystemMetricsProxy* proxy) {
    return [proxy](const MetricSettings& settings) -> OutputPipeline {
        auto serializer = std::make_shared<JsonSerializer>(settings);
        return [serializer, proxy](const std::list<SystemMetrics>& results) {
            if (!proxy) return;
            nlohmann::json json_data = nlohmann::json::array();
            for (const auto& m : results) {
                json_data.push_back(serializer->serialize(m));
            }
            // std::cout << json_data.dump() ;
            proxy->updateData(json_data);
        };
    };
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    SystemMetricsProxy proxy;
    QQmlApplicationEngine engine;

    // Set context property before loading QML
    engine.rootContext()->setContextProperty("systemData", &proxy);

    // Register factory first
    ParsedConfig::register_pipeline(OutputMode::WIDGETS, widget_factory(&proxy));

    ProgramOptions options = parse_cli(argc, argv);
    ParsedConfig config = build_config_from_options(options);
    config.set_output_mode(OutputMode::WIDGETS);

    auto controller = std::make_unique<Controller>();
    controller->initialize(config);

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() { controller->tick(); });
    timer.start(1000);

    // Ensure path matches CMake qt_add_qml_module URI if used
    const QUrl url(QStringLiteral("qrc:/qt/qml/main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty()) return -1;

    return app.exec();
}
