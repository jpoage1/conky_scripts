// src/core/runner_context.cpp
#include "runner_context.hpp"

#include "pcn.hpp"
#include "system_metrics_proxy.hpp"
#include "types.hpp"

RunnerContext::RunnerContext(int argc, char** argv, const ControllerPtr& c)
    : argc(argc), argv(argv), controller(c) {}
