// tests/lws_reactor_test.cpp
#include <gtest/gtest.h>
#include "mock_controller.hpp"
#include "lws_main.hpp"
#include "runner_context.hpp"

using ::testing::Return;
using ::testing::Exactly;

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
