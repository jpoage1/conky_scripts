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
TEST_F(StabilityValidation, ProcessIOPopulated) {
  // 1. MANUALLY populate the list with the current process
  ProcessInfo mock_proc;
  mock_proc.pid = getpid(); // Audit ourselves
  mock_proc.name = "unit_tests";
  metrics.top_processes_real_cpu.push_back(mock_proc);

  // 2. Run the IO expansion logic
  // Ensure you use the specific class name you implemented for IO/FD auditing
  ProcessPollingTask io_task(provider, metrics, context);
  io_task.take_new_snapshot();
  io_task.calculate();

  // 3. Verify the vector is no longer empty and data is populated
  ASSERT_FALSE(metrics.top_processes_real_cpu.empty());

  // Check that open_fds is now > 0 (it was 0 in your JSON output)
  EXPECT_GT(metrics.top_processes_real_cpu[0].open_fds, 0);
}
