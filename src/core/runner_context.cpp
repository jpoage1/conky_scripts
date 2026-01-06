// src/core/runner_context.cpp
#include "runner_context.hpp"
#include "system_metrics_proxy.hpp"
#include "pcn.hpp"
#include "types.hpp"

RunnerContext::RunnerContext(int argc, char** argv, ControllerPtr& c) : argc(argc), argv(argv), controller(c) {}
void RunnerContext::attach(SystemMetricsProxyPtr& proxy) {
    this->proxy = proxy;
}  

