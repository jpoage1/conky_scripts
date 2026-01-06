#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include "controller.hpp"
#include "configuration_builder.hpp"
#include "cli_parser.hpp"
#include "system_metrics_qt_proxy.hpp"
#include "system_metrics_proxy.hpp"
#include "json_serializer.hpp"
#include "types.hpp"
#include "parsed_config.hpp"
#include "telemetry.hpp"
#include "runner_context.hpp"
#include "qt.hpp"
#include <unistd.h>

void register_qt_pipeline() {
    auto proxy = std::make_shared<SystemMetricsQtProxy>();
    PipelineEntry pipeline{"qt", qt_widget_factory(proxy.get()), qt_main, proxy};
    ParsedConfig::register_pipeline(pipeline);
}

int qt_main(const RunnerContext &ctx) {
    // 1. Fork the process
    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Fork failed" << std::endl;
        return 1;
    }

    // 2. Terminate the parent process
    if (pid > 0) {
        // Parent exits, shell returns prompt to user
        return 0;
    }
    
    QApplication app(ctx.argc, ctx.argv);
    QQmlApplicationEngine engine;

    SystemMetricsProxyPtr proxy_shared = ctx.controller->get_proxy();
    // Set context property before loading QML
    if (!proxy_shared) {
        throw std::runtime_error("Qt Main: SystemMetricsProxy is null. Pipeline registration may have failed.");
    }

    QObject* qobject_proxy = dynamic_cast<QObject*>(proxy_shared.get());

    if (!qobject_proxy) {
        // This will trigger if SystemMetricsProxy does not inherit from QObject
        SPDLOG_ERROR("Qt Main: Type verification failed. Proxy does not inherit from QObject.");
        throw std::runtime_error("Qt Main: Invalid proxy type detected.");
    }
        engine.rootContext()->setContextProperty("systemData", qobject_proxy);
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() { ctx.controller->tick(); });
    timer.start(1000);

    engine.addImportPath("qrc:/qt/qml");
    
    // Ensure path matches CMake qt_add_qml_module URI if used
    const QUrl url(QStringLiteral("qrc:/qt/qml/main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty()) return -1;

    return app.exec();
}
