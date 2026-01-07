// tests/lws_reactor_test.cpp
#include "lws_main.hpp"
#include "mock_controller.hpp"
#include "runner_context.hpp"
#include <gtest/gtest.h>

using ::testing::Exactly;
using ::testing::Return;

namespace telemetry {

TEST(LwsReactorTest, ExecutesExpectedCycle) {
  auto mock = std::make_shared<MockController>();
  RunnerContext ctx;
  ctx.controller = mock;

  // Simulate 2 cycles then exit
  EXPECT_CALL(*mock, is_persistent())
      .WillOnce(Return(true))
      .WillOnce(Return(true))
      .WillOnce(Return(false));

  // Tick and sleep should occur for each true persistence check
  EXPECT_CALL(*mock, tick()).Times(2);
  EXPECT_CALL(*mock, sleep()).Times(2);

  // Act
  int result = lws_main(ctx);

  // Assert
  ASSERT_EQ(result, 0);
}

}; // namespace telemetry
