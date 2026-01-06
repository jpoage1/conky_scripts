#include "json_definitions.hpp"
#include "metrics.hpp"
#include "mock_context.hpp"
#include "polling.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

class StabilityValidation : public MockLocalContext {};

TEST_F(StabilityValidation, JsonRoundTrip) {
  metrics.stability.memory_fragmentation_index = 0.45;
  metrics.stability.file_descriptors_allocated = 1024;

  nlohmann::json j = metrics; // Trigger to_json
  SystemMetrics restored_metrics;
  restored_metrics = j.get<SystemMetrics>(); // Trigger from_json

  EXPECT_EQ(restored_metrics.stability.memory_fragmentation_index, 0.45);
  EXPECT_EQ(restored_metrics.stability.file_descriptors_allocated, 1024);
}
