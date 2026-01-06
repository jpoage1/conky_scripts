
#include "controller.hpp"

#ifdef OUTPUT_MODE_QT
#include "qt.hpp"
#endif

#ifdef OUTPUT_MODE_JSON
#include "output_mode_json.hpp"
#endif

#ifdef OUTPUT_MODE_CONKY
#include "output_mode_conky.hpp"
#endif

int main(int argc, char* argv[]) {
  const RunnerContext context;

  #ifdef OUTPUT_MODE_QT
    register_qt_pipeline();
  #endif

  #ifdef OUTPUT_MODE_JSON
    register_json_pipeline()
  #endif OUTPUT_MODE_JSON

  #ifdef OUTPUT_MODE_CONKY
    register_conky_pipeline()
  #endif
  
  const ControllerPtr controller = initialize(argc, argv);
  const RunnerContext attach(argc, argv, controller);
  return controller.main(context);
};
