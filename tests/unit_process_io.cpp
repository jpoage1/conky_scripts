// tests/unit_process_io.cpp
#include "mock_context.hpp"
#include "polling.hpp"
#include "processinfo.hpp"
#include <gtest/gtest.h>

class ProcessIOTest : public MockLocalContext {};

TEST_F(ProcessIOTest, AuditSelfProcess) {
  ProcessPollingTask task(provider, metrics, context);

  ProcessInfo self;
  self.pid = getpid(); // Audit this test process
  std::vector<ProcessInfo> list = {self};

  task.audit_process_list(list);

  // Every running process should have at least 3 FDs (stdin, out, err)
  EXPECT_GT(list[0].open_fds, 2);
  // This process likely has read some bytes during init
  EXPECT_GE(list[0].io_read_bytes, 0);
}
