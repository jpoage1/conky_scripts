// includes/telemetry.hpp
#ifndef TELEMETRY_HPP
#define
#include <memory>

#include "types.hpp"

namespace telemetry {
class Controller;

ControllerPtr initialize(int argc, char *argv[]);

}; // namespace telemetry

#endif
