#include "mock_context.hpp"
#include "polling.hpp"
#include "system_stability.hpp"
#include <gtest/gtest.h>

class StabilityValidation : public MockLocalContext {};

// Validate that PSI metrics are within the 0.0 - 100.0 range
TEST_F(StabilityValidation, PSIRangesAreSane) {
  SystemStabilityPollingTask task(provider, metrics, context);
  task.take_new_snapshot();
  task.calculate();

  EXPECT_GE(metrics.stability.memory_pressure_some, 0.0);
  EXPECT_LE(metrics.stability.memory_pressure_some, 100.0);
  EXPECT_GE(metrics.stability.io_pressure_full, 0.0);
  EXPECT_LE(metrics.stability.io_pressure_full, 100.0);
}

// Ensure File Descriptor allocation does not exceed system max
TEST_F(StabilityValidation, FDAuditConsistency) {
  SystemStabilityPollingTask task(provider, metrics, context);
  task.take_new_snapshot();

  EXPECT_GT(metrics.stability.file_descriptors_max, 0);
  EXPECT_LE(metrics.stability.file_descriptors_allocated,
            metrics.stability.file_descriptors_max);
}

// Validate Process IO expansion actually populates values
TEST_F(StabilityValidation, ProcessIOExpansionPopulated) {
  // Manually push a known PID (the current process) into top_processes
  ProcessInfo self;
  self.pid = getpid();
  metrics.top_processes_real_cpu.push_back(self);

  ProcessPollingTask task(provider, metrics, context);
  task.take_new_snapshot();

  // The first process in the list should now have an FD count > 0
  EXPECT_GT(metrics.top_processes_real_cpu[0].open_fds, 0);
}
