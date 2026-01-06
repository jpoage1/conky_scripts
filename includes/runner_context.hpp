// src/core/runner_context.cpp
#ifndef RUNNER_CONTEXT_HPP
#define RUNNER_CONTEXT_HPP

#include "pcn.hpp"
#include "types.hpp"

class SystemMetricsProxy;

struct RunnerContext {
  int& argc;
  char** const argv;
  const ControllerPtr& controller;
  std::shared_ptr<SystemMetricsProxy> proxy = nullptr;
  RunnerContext(int argc, char** argv, const ControllerPtr& c);
};

#endif
