#ifndef MOCK_CONTROLLER_HPP
#define MOCK_CONTROLLER_HPP

#include "controller.hpp"
#include <gmock/gmock.h>

class MockController : public Controller {
public:
  MockController() = default;
  virtual ~MockController() = default;

  // Core execution hooks
  MOCK_METHOD(void, tick, (), (override));
  MOCK_METHOD(bool, is_persistent, (), (const, override));
  MOCK_METHOD(void, sleep, (), (override));

  // Initialization and Entry Points
  MOCK_METHOD(void, initialize, (ParsedConfig & config), (override));
  MOCK_METHOD(int, main, (const RunnerContext &context), (override));
  MOCK_METHOD(SystemMetricsProxyPtr, get_proxy, (), (override));
};

#endif
