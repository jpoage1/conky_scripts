#pragma once

#include "data.h"
#include "metrics.hpp"
#include "pcn.h"

/**
 * @brief An interface for any task that requires two snapshots over
 * time to calculate a rate (e.g., CPU, Network).
 */
class IPollingTask {
 protected:
  DataStreamProvider& provider;
  SystemMetrics& metrics;
  std::string name;

 public:
  /**
   * @brief Constructs a task by storing references to its context.
   * * We use an initializer list (the ': ...') because references
   * MUST be initialized, they cannot be assigned later.
   */
  IPollingTask(DataStreamProvider& _provider, SystemMetrics& _metrics)
      : provider(_provider), metrics(_metrics) {}
  virtual ~IPollingTask() = default;
  std::string get_name() { return name; }
  /**
   * @brief Take the "Time 1" (T1) snapshot and store it internally.
   */
  virtual void take_snapshot_1() = 0;

  /**
   * @brief Take the "Time 2" (T2) snapshot and store it internally.
   */
  virtual void take_snapshot_2() = 0;

  /**
   * @brief Use the stored T1 and T2 snapshots to perform the
   * calculation and save the result into the metrics object.
   */
  virtual void calculate(double time_delta_seconds) = 0;
};

using PollingTaskList = std::vector<std::unique_ptr<IPollingTask>>;
