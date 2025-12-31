// tests/mock_context.hpp
#include <gtest/gtest.h>

#include "corestat.hpp"
#include "data_local.hpp"
#include "data_ssh.hpp"
#include "metrics.hpp"
#include "context.hpp"

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
