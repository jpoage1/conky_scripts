// tests/unit_corestat.cpp
#include <gtest/gtest.h>
#include "corestat.hpp"
#include "mock_context.hpp"
#include "polling.hpp"

class CpuCoverageTest : public MockLocalContext {
};

// Test: 100% coverage for malformed/mismatched input sizes
TEST_F(CpuCoverageTest, MismatchedSnapshotSizes) {
    CpuPollingTask task(provider, metrics, context);
    
    CpuSnapshot s;
    s.user = 100; s.idle = 100;
    
    // Simulation: T1 has 4 cores, T2 has 2 cores
    task.prev_snapshots = {s, s, s, s};
    task.current_snapshots = {s, s};
    
    ASSERT_NO_THROW(task.calculate());
    // Verification: metrics should reflect the most recent snapshot size
    EXPECT_EQ(metrics.cores.size(), 2);
}

// Test: Memory Overflow / Counter Wrap handling
TEST_F(CpuCoverageTest, CounterWrapHandling) {
    CpuPollingTask task(provider, metrics, context);
    
    CpuSnapshot t1, t2;
    t1.user = 1000; t1.idle = 1000;
    t2.user = 500;  t2.idle = 500; // Counter wrapped/reset
    
    task.prev_snapshots = {t1};
    task.current_snapshots = {t2};
    
    task.calculate();
    // Should default to 0% usage, 100% idle on wrap to prevent insane values
    EXPECT_FLOAT_EQ(metrics.cores[0].total_usage_percent, 0.0f);
    EXPECT_FLOAT_EQ(metrics.cores[0].idle_percent, 100.0f);
}

// Test: Aggregate Accuracy (Index 0)
TEST_F(CpuCoverageTest, AggregateAccuracy) {
    CpuPollingTask task(provider, metrics, context);
    
    CpuSnapshot t1_agg{0,0,0,0,0,0,0,0};
    CpuSnapshot t2_agg{100, 0, 100, 200, 0, 0, 0, 0}; // 400 total, 200 work
    
    task.prev_snapshots = {t1_agg};
    task.current_snapshots = {t2_agg};
    
    task.calculate();
    EXPECT_NEAR(metrics.cores[0].total_usage_percent, 50.0f, 0.01f);
}


TEST_F(CpuCoverageTest, AggregateIsZeroIndex) {
    CpuPollingTask task(provider, metrics, context);
    std::string mock_stat = 
        "cpu1 100 0 0 100 0 0 0 0\n"
        "cpu  200 0 0 200 0 0 0 0\n"; // Aggregate out of order
    std::istringstream iss(mock_stat);
    
    auto snaps = task.read_data(iss);
    task.prev_snapshots = { {0}, {0} };
    task.current_snapshots = snaps;
    
    task.calculate();
    
    ASSERT_GT(metrics.cores.size(), 0);
    EXPECT_EQ(metrics.cores[0].core_id, 0); // Must be 0
}