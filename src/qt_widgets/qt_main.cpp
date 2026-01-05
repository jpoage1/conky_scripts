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
#include "telemetry.hpp"
#include "qt.hpp"
#include "telemetry.hpp"
#include <unistd.h>


int main(int argc, char* argv[]) {
    std::unique_ptr<Controller> controller = initialize(argc, argv);

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
    
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    SystemMetricsProxy proxy;

    // Set context property before loading QML
    engine.rootContext()->setContextProperty("systemData", &proxy);

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() { controller->tick(); });
    timer.start(1000);

    engine.addImportPath("qrc:/qt/qml");
    
    // Ensure path matches CMake qt_add_qml_module URI if used
    const QUrl url(QStringLiteral("qrc:/qt/qml/main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty()) return -1;

    return app.exec();
}
