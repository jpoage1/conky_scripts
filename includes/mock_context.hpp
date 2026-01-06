// tests/mock_context.hpp
#ifndef MOCK_CONTEXT_HPP
#define MOCK_CONTEXT_HPP

#include <gtest/gtest.h>

#include "context.hpp"
#include "corestat.hpp"
#include "data_local.hpp"
#include "data_ssh.hpp"
#include "metrics.hpp"

class MockLocalContext : public ::testing::Test {
 protected:
  MetricsContext context;
  SystemMetrics metrics;
  LocalDataStreams provider;
  MockLocalContext() : context(), metrics(context) {}
};

class MockProcContext : public ::testing::Test {
 protected:
  MetricsContext context;
  SystemMetrics metrics;
  ProcDataStreams provider;
  MockProcContext() : context(), metrics(context) {}
};
#endif
