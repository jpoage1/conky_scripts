#include <unistd.h>

#include "cli_parser.hpp"
#include "configuration_builder.hpp"
#include "controller.hpp"
#include "json_serializer.hpp"
#include "parsed_config.hpp"
#include "qt.hpp"
#include "runner_context.hpp"
#include "system_metrics_proxy.hpp"
#include "system_metrics_qt_proxy.hpp"
#include "telemetry.hpp"
#include "types.hpp"
#include <QApplication>
#include <QDirIterator>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

int qt_main(const RunnerContext &ctx) {
  // std::cerr << "Starting qt main" << std::endl;
  /*
   // 1. Fork the process
  pid_t pid = fork();

  if (pid < 0) {
    std::cerr << "Fork failed" << std::endl;
    return 1;
  }

  // 2. Terminate the parent process
  if (pid > 0) {
    std::cerr << "Terminating parent process" << std::endl;
    // Parent exits, shell returns prompt to user
    _exit(0);
  }
  std::cerr << "Child process" << std::endl;
  setsid();
  freopen("/tmp/telemetry_child.log", "a", stdout);
  freopen("/tmp/telemetry_child.log", "a", stderr);
*/
  Q_INIT_RESOURCE(resources);
  QApplication app(ctx.argc, ctx.argv);

  // Verify Display Environment
  if (getenv("DISPLAY") == nullptr && getenv("WAYLAND_DISPLAY") == nullptr) {
    // Since we redirected stderr, we should log this via spdlog
    SPDLOG_ERROR("No display detected in daemonized child.");
    return 1;
  }

  QQmlApplicationEngine engine;

#ifdef DEBUG_TRACE
  // Debug: Print all available resources to stderr to verify they exist
  QDirIterator it(":", QDirIterator::Subdirectories);
  while (it.hasNext()) {
    qDebug() << it.next();
#endif

    SystemMetricsProxyPtr proxy_shared = ctx.controller->get_proxy();
    // Set context property before loading QML
    if (!proxy_shared) {
      SPDLOG_WARN(
          "Qt Main: SystemMetricsProxy is null. Pipeline registration may have "
          "failed.");
      // return 1;
    }

    QObject *qobject_proxy = dynamic_cast<QObject *>(proxy_shared.get());

    if (!qobject_proxy) {
      // This will trigger if SystemMetricsProxy does not inherit from QObject
      SPDLOG_WARN(
          "Qt Main: Type verification failed. Proxy does not inherit from "
          "QObject.");
      // throw std::runtime_error("Qt Main: Invalid proxy type detected.");
      SPDLOG_WARN("Qt Main: Invalid proxy type detected.");
    }

    SPDLOG_TRACE("Starting qml root context");
    engine.rootContext()->setContextProperty("systemData", qobject_proxy);
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout,
                     [&]() { ctx.controller->tick(); });
    timer.start(1000);

    engine.addImportPath("qrc:/qt/qml");

    // Ensure path matches CMake qt_add_qml_module URI if used
    const QUrl url(QStringLiteral("qrc:/qt/qml/main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty()) {
      SPDLOG_ERROR("root objects is empty");
      return -1;
    }

    return app.exec();
  }
