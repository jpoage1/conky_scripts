// polling_task.h
#pragma once

#include "metrics.hpp"
#include "provider.hpp"

using CpuSnapshotList = std::vector<CpuSnapshot>;
using NetworkSnapshotMap = std::map<std::string, NetworkSnapshot>;
using DiskIoSnapshotMap = std::map<std::string, DiskIoSnapshot>;

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
  IPollingTask(DataStreamProvider& _provider, SystemMetrics& _metrics);

  //   IPollingTask(DataStreamProvider& _provider, SystemMetrics& _metrics)
  //       : provider(_provider), metrics(_metrics) {}

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
  virtual void commit() = 0;
};
class CpuPollingTask : public IPollingTask {
 private:
  CpuSnapshotList t1_snapshots;
  CpuSnapshotList t2_snapshots;

 public:
  CpuPollingTask(DataStreamProvider&, SystemMetrics&);
  void take_snapshot_1() override;
  void take_snapshot_2() override;
  void calculate(double /*time_delta_seconds*/) override;
  void commit() override {};
  CpuSnapshotList read_data(std::istream&);
};
using CpuPollingTaskPtr = std::unique_ptr<CpuPollingTask>;

class NetworkPollingTask : public IPollingTask {
 private:
  NetworkSnapshotMap t1_snapshot;
  NetworkSnapshotMap t2_snapshot;

 public:
  NetworkPollingTask(DataStreamProvider&, SystemMetrics&);
  void take_snapshot_1() override;
  void take_snapshot_2() override;
  void calculate(double time_delta_seconds) override;
  void commit() override {};

  NetworkSnapshotMap read_data(std::istream&);
};
using NetworkPollingTaskPtr = std::unique_ptr<NetworkPollingTask>;

class DiskPollingTask : public IPollingTask {
 private:
  DiskIoSnapshotMap t1_snapshots;
  DiskIoSnapshotMap t2_snapshots;

 public:
  DiskPollingTask(DataStreamProvider&, SystemMetrics&);
  void take_snapshot_1() override;
  void take_snapshot_2() override;

  void calculate(double time_delta_seconds) override;
  void commit() override;

  DiskIoSnapshotMap read_data(std::istream&);
};
using DiskPollingTaskPtr = std::unique_ptr<DiskPollingTask>;
