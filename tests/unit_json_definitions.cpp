#ifdef OUTPUT_MODE_JSON
#include "json_definitions.hpp"
#include "json_serializer.hpp"
#include "metrics.hpp"
#include "mock_context.hpp"
#include "polling.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

class StabilityValidation : public MockLocalContext {};
TEST_F(StabilityValidation, JsonRoundTrip) {
  metrics.stability.memory_fragmentation_index = 0.45;
  metrics.stability.file_descriptors_allocated = 5000;

  // 1. Serialize to JSON object
  nlohmann::json j = metrics;

  // 2. Explicitly verify it's an object before proceeding
  ASSERT_TRUE(j.is_object())
      << "Serialization failed: Expected JSON object, got " << j.type_name();

  // 3. Reconstruct using the parameterized constructor
  SystemMetrics restored(context);

  // 4. Use get_to to trigger your from_json logic
  j.get_to(restored);

  EXPECT_NEAR(restored.stability.memory_fragmentation_index, 0.45, 0.001);
  EXPECT_EQ(restored.stability.file_descriptors_allocated, 5000);
}
#endif
