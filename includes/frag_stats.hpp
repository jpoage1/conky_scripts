// includes/frag_stats.hpp
#ifndef FRAG_STATS_HPP
#define FRAG_STATS_HPP
#include "polling.hpp"

class SystemMetrics;
class DataStreamProvider;
class MetricsContext;

struct FragStats {
  int node_id;
  std::string zone;
  std::vector<int> free_chunks_by_order; // Order 0 (4k) to Order 10 (4M)
};

class MemoryFragmentationTask : public IPollingTask {
public:
  MemoryFragmentationTask(DataStreamProvider &p, SystemMetrics &m,
                          MetricsContext &ctx);

  void configure() override;
  void take_initial_snapshot() override;

  void take_new_snapshot() override;
  void calculate() override;
  void commit() override;
};
#endif
