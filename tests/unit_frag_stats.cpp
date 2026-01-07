// tests/unit_frag.cpp
#include "frag_stats.cpp"
#include <gtest/gtest.h>

namespace telemetry {

TEST(FragmentationTest, ContiguousMemory) {
  // Large blocks available at order 10
  std::vector<int> contiguous = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10};
  double index = calculate_fragmentation_index(contiguous);

  // Contiguous memory should result in a low index (near 0)
  EXPECT_LT(index, 0.1);
}

TEST(FragmentationTest, HighlyFragmentedMemory) {
  // Only small 4K pages available (Order 0)
  std::vector<int> fragmented = {1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  double index = calculate_fragmentation_index(fragmented);

  // Fully fragmented memory should result in index 1.0
  EXPECT_NEAR(index, 1.0, 0.001);
}

TEST(FragmentationTest, EmptyBuddyInfo) {
  std::vector<int> empty = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  double index = calculate_fragmentation_index(empty);

  EXPECT_EQ(index, 0.0); // Should handle division by zero safely
}

}; // namespace telemetry
