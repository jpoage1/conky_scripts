#include <unistd.h>

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

#include "cli_parser.hpp"
#include "configuration_builder.hpp"
#include "controller.hpp"
#include "json_serializer.hpp"
#include "parsed_config.hpp"
#include "qt.hpp"
#include "runner_context.hpp"
#include "system_metrics_qt_proxy.hpp"
#include "types.hpp"

static bool qt_registered = []() {
  register_qt_pipeline();
  return true;
}();

PipelineFactory qt_widget_factory(SystemMetricsProxy* proxy) {
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
  SystemMetricsProxy proxy;
  QQmlApplicationEngine engine;

  // Set context property before loading QML
  engine.rootContext()->setContextProperty("systemData", &proxy);

  ProgramOptions options = parse_cli(argc, argv);
  ParsedConfig config = build_config_from_options(options);
  ParsedConfig::register_pipeline("qt", OutputMode::QT_WIDGETS,
                                  widget_factory(&proxy));

  auto controller = std::make_unique<Controller>();
  controller->initialize(config);

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
