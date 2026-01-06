// src/systemdata/frag_stats.cpp
#include "frag_stats.hpp"

double calculate_fragmentation_index(const std::vector<int> &chunks) {
  double total_free_pages = 0;
  double weighted_free_pages = 0;

  for (size_t order = 0; order < chunks.size(); ++order) {
    // Number of pages in this chunk = 2^order
    double pages_in_chunk = std::pow(2, order);
    double pages_at_this_order = chunks[order] * pages_in_chunk;

    total_free_pages += pages_at_this_order;

    // Weighting: Higher orders are "better" (less fragmented).
    // We subtract the availability of large blocks from the index.
    weighted_free_pages += (pages_at_this_order * (order / 10.0));
  }

  if (total_free_pages == 0)
    return 0.0;

  // Result is between 0 (contiguous) and 1 (fragmented)
  return 1.0 - (weighted_free_pages / total_free_pages);
}

MemoryFragmentationTask::MemoryFragmentationTask(DataStreamProvider &p,
                                                 SystemMetrics &m,
                                                 MetricsContext &ctx)
    : IPollingTask(p, m, ctx) {
  name = "Memory Fragmentation";
}

void MemoryFragmentationTask::configure() {}
void MemoryFragmentationTask::take_initial_snapshot() { take_new_snapshot(); }

void MemoryFragmentationTask::take_new_snapshot() {
  std::ifstream buddy("/proc/buddyinfo");
  std::string line;
  // Parse: Node 0, zone Normal  1 2 3 4 5 6 7 8 9 10 11
  while (std::getline(buddy, line)) {
    std::istringstream ss(line);
    std::string discard, zone;
    int node;
    ss >> discard >> node >> discard >> zone;

    int count;
    long high_order_sum = 0;
    long total_chunks = 0;
    for (int i = 0; i < 11; ++i) {
      ss >> count;
      total_chunks += count;
      if (i > 7)
        high_order_sum += count; // Tracking large contiguous chunks
    }
    // If high_order_sum is low relative to total_chunks, fragmentation is
    // high.
  }
}
void MemoryFragmentationTask::calculate() {
  std::ifstream buddy("/proc/buddyinfo");
  std::string line;
  double global_index_sum = 0.0;
  int zones_counted = 0;

  while (std::getline(buddy, line)) {
    std::istringstream ss(line);
    std::string discard, zone;
    int node;
    // Format: Node 0, zone Normal  1 2 3 4 ...
    ss >> discard >> node >> discard >> zone;

    std::vector<int> chunks;
    int val;
    while (ss >> val) {
      chunks.push_back(val);
    }

    if (!chunks.empty()) {
      global_index_sum += calculate_fragmentation_index(chunks);
      zones_counted++;
    }
  }

  if (zones_counted > 0) {
    // Store the average fragmentation across all zones
    metrics.stability.memory_fragmentation_index =
        global_index_sum / zones_counted;
  }
}
void MemoryFragmentationTask::commit() {}
