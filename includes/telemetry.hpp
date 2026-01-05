// includes/telemetry.hpp
#include <memory>

class Controller;

using ControllerPtr = std::unique_ptr<Controller>;
ControllerPtr initialize(int argc, char* argv[]);
