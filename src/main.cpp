
#include "controller.hpp"
#include "log.hpp"
#include "runner_context.hpp"
#include "telemetry.hpp"

#ifdef OUTPUT_MODE_QT
#include "qt.hpp"
#endif

#ifdef OUTPUT_MODE_JSON
#include "output_mode_json.hpp"
#endif

#ifdef OUTPUT_MODE_LIBWEBSOCKETS
#include "libwebsockets.hpp"
#endif

#ifdef OUTPUT_MODE_CONKY
#include "output_mode_conky.hpp"
#endif

int main(int argc, char *argv[]) {
  int pipeline_count = 0;
#define REGISTER_OUTPUT_MODE(OUTPUT_MODE_NAME, OUTPUT_MODE_PIPELINE)           \
  do {                                                                         \
    SPDLOG_DEBUG("Available output mode: {}", OUTPUT_MODE_NAME);               \
    OUTPUT_MODE_PIPELINE();                                                    \
    pipeline_count++;                                                          \
  } while (0);

#ifdef OUTPUT_MODE_JSON
  REGISTER_OUTPUT_MODE("json", register_json_pipeline);
#endif

#ifdef OUTPUT_MODE_CONKY
  REGISTER_OUTPUT_MODE("conky", register_conky_pipeline);
#endif

#ifdef OUTPUT_MODE_QT
  REGISTER_OUTPUT_MODE("qt", register_qt_pipeline);
#endif

#ifdef OUTPUT_MODE_LIBWEBSOCKETS
  REGISTER_OUTPUT_MODE("libwebsockets", register_libwebsockets_pipeline);
#endif

  if (pipeline_count == 0) {
    SPDLOG_ERROR("No pipelines were enabled.");
    return 1;
  }

  const ControllerPtr controller = initialize(argc, argv);
  if (!controller) {
    SPDLOG_ERROR(""); // fixme
    return 1;
  }
  const RunnerContext context(argc, argv, controller);
  SPDLOG_TRACE("Calling main");

  return controller->main(context);
}
