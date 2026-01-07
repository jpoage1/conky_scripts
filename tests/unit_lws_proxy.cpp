// tests/unit_lws_proxy.cpp
#include "lws_proxy.hpp"
#include <gtest/gtest.h>

namespace libwebsockets {

TEST(LwsProxyTest, DISABLED_DataTransitIntegrity) {
  SystemMetricsLwsProxy proxy;
  nlohmann::json test_data = {{"cpu", 50}, {"mem", 2048}};

  proxy.updateData(test_data);

  ASSERT_TRUE(proxy.ready());
  ASSERT_EQ(proxy.consume(), test_data.dump());
  ASSERT_FALSE(proxy.ready()); // Should be cleared after consumption
}

}; // namespace libwebsockets
