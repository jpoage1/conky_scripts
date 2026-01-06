// tests/unit_json_order.cpp
#ifdef OUTPUT_MODE_JSON
#include "json_serializer.hpp"
#include "metric_settings.hpp"
#include "mock_context.hpp"
#include "polling.hpp"
#include "processinfo.hpp"
#include <gtest/gtest.h>

class JsonOrderTest : public MockLocalContext {};

TEST_F(JsonOrderTest, FeatureGatingMismatchFix) {
  MetricSettings settings;
  settings.enable_sysinfo = false;
  settings.enable_cpuinfo = true; // CPU ON, SYS OFF

  metrics.cores.push_back({0, 50.0f, 0.0f, 0.0f, 0.0f, 50.0f, 50.0f});

  JsonSerializer serializer(settings);
  nlohmann::json result = serializer.serialize(metrics);

  EXPECT_FALSE(result.contains("sys_name"));
  EXPECT_TRUE(result.contains(
      "cores")); // Verification: cores now gated by enable_cpuinfo
}
#endif
