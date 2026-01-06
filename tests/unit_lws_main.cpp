// tests/lws_main_test.cpp
#include "libwebsockets.hpp" // Fix: Must include the header for lws_main
#include "mock_controller.hpp"
#include "runner_context.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Use unique_ptr to match the ControllerPtr alias
typedef std::unique_ptr<MockController> MockControllerPtr;
using namespace libwebsockets;

TEST(LwsMainTest, LoopExecutesTickThenExits) {
  // 1. Prepare dummy argc/argv
  int argc = 1;
  char *argv[] = {(char *)"test_bin", nullptr};

  // 2. Instantiate the mock as a unique_ptr
  auto mock_controller = std::make_unique<MockController>();

  // Hold a raw pointer to set expectations before moving it into the context
  MockController *raw_mock = mock_controller.get();

  // 3. Set Expectations on the raw pointer
  EXPECT_CALL(*raw_mock, is_persistent())
      .WillOnce(testing::Return(true))
      .WillOnce(testing::Return(false));

  EXPECT_CALL(*raw_mock, tick()).Times(1);
  EXPECT_CALL(*raw_mock, sleep()).Times(1);

  // 4. Move the unique_ptr into a temporary variable to match the reference
  // requirement In a real scenario, the ControllerPtr would be owned by the
  // caller.
  ControllerPtr controller_owner = std::move(mock_controller);

  // 5. Initialize Context with arguments
  RunnerContext ctx(argc, argv, controller_owner);

  // Act
  int result = lws_main(ctx);

  // Assert
  ASSERT_EQ(result, 0);
}
