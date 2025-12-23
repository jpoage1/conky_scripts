// tests/main.cpp
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // Directive: Set level to TRACE to enable greppable anchors
  spdlog::set_level(spdlog::level::trace);

  // Optional: Force pattern to match production logs for easier grepping
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

  return RUN_ALL_TESTS();
}
