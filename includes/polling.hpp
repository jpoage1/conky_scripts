// polling_task.h
#pragma once

#include "metrics.hpp"
#include "processinfo.hpp"
#include "provider.hpp"

using CpuSnapshotList = std::vector<CpuSnapshot>;
using NetworkSnapshotMap = std::map<std::string, NetworkSnapshot>;
using DiskIoSnapshotMap = std::map<std::string, DiskIoSnapshot>;

struct ProcessSnapshot {
  long pid;
  long vmRssKb;
  long cumulative_cpu_jiffies;  // The raw reading from /proc/[pid]/stat
  std::string name;
};

using ProcessSnapshotList = std::vector<ProcessSnapshot>;

struct ProcessRawSnapshot {
  std::string name;
  long vmRssKb = 0;
  long cumulative_cpu_time = 0;  // Jiffies (Local) or Seconds*100 (SSH)
  unsigned long long start_time =
      0;  // Field 22 (Jiffies) or Elapsed Seconds (SSH)
};

using ProcessSnapshotMap = std::map<long, ProcessRawSnapshot>;  // Key = PID

class IPollingTask {
 protected:
  DataStreamProvider& provider;
  SystemMetrics& metrics;
  std::string name;
  std::chrono::steady_clock::time_point timestamp;
  double time_delta_seconds;

 public:
  /**
   * @brief Constructs a task by storing references to its context.
   * * We use an initializer list (the ': ...') because references
   * MUST be initialized, they cannot be assigned later.
   */
  IPollingTask(DataStreamProvider& provider, SystemMetrics& metrics,
               MetricsContext&);
  //   : provider(provider), metrics(metrics) {};

  //   IPollingTask(DataStreamProvider& _provider, SystemMetrics& _metrics)
  //       : provider(_provider), metrics(_metrics) {}

  virtual ~IPollingTask() = default;
  std::string get_name() { return name; }

  void set_delta_time();
  void set_timestamp();

  virtual void configure() = 0;
  /**
   * @brief Take the "Time 1" (T1) snapshot and store it internally.
   */
  virtual void take_initial_snapshot() = 0;

  /**
   * @brief Take the "Time 2" (T2) snapshot and store it internally.
   */
  virtual void take_new_snapshot() = 0;

  /**
   * @brief Use the stored T1 and T2 snapshots to perform the
   * calculation and save the result into the metrics object.
   */
  virtual void calculate() = 0;
  virtual void commit() = 0;
};
class CpuPollingTask : public IPollingTask {
 private:
  CpuSnapshotList prev_snapshots;
  CpuSnapshotList current_snapshots;

 public:
  CpuPollingTask(DataStreamProvider&, SystemMetrics&, MetricsContext&);
  void configure() override {};
  void take_initial_snapshot() override;
  void take_new_snapshot() override;
  void calculate() override;
  void commit() override;
  CpuSnapshotList read_data(std::istream&);
};
using CpuPollingTaskPtr = std::unique_ptr<CpuPollingTask>;

class NetworkPollingTask : public IPollingTask {
 private:
  NetworkSnapshotMap prev_snapshot;
  NetworkSnapshotMap current_snapshot;

 public:
  NetworkPollingTask(DataStreamProvider&, SystemMetrics&, MetricsContext&);
  void configure() override {};
  void take_initial_snapshot() override;
  void take_new_snapshot() override;
  void calculate() override;
  void commit() override;

  NetworkSnapshotMap read_data(std::istream&);
};
using NetworkPollingTaskPtr = std::unique_ptr<NetworkPollingTask>;

class DiskPollingTask : public IPollingTask {
 private:
  DiskIoSnapshotMap prev_snapshots;
  DiskIoSnapshotMap current_snapshots;
  // A set of the kernel device names we actually care about (e.g., "dm-0",
  // "sda1")
  std::set<std::string> target_kernel_names;
  // A map to link logical paths to kernel names (e.g., "/dev/vg0/lv0" ->
  // "dm-0")
  std::map<std::string, DeviceInfo*> kernel_to_device_map;
  DevicePaths load_device_paths(const std::string& config_file);
  std::set<std::string> allowed_io_devices;
  DiskStatConfig config;

 public:
  DiskPollingTask(DataStreamProvider&, SystemMetrics&, MetricsContext&);
  void configure() override;
  void take_initial_snapshot() override;
  void take_new_snapshot() override;

  void calculate() override;
  void commit() override;

  DiskIoSnapshotMap read_data(std::istream&);
};
using DiskPollingTaskPtr = std::unique_ptr<DiskPollingTask>;
class ProcessPollingTask : public IPollingTask {
 private:
  long unsigned int process_count = 10;
  std::vector<std::string> ignore_list;
  bool only_user_processes = true;
  ProcessSnapshotMap prev_snapshots;
  ProcessSnapshotMap current_snapshots;
  std::vector<std::function<void(std::vector<ProcessInfo>&)>> output_pipeline;
  void populate_top_ps(std::vector<ProcessInfo>& source,
                       std::vector<ProcessInfo>& dest, SortMode mode);

 public:
  ProcessPollingTask(DataStreamProvider&, SystemMetrics&, MetricsContext&);
  void configure() override {};
  void take_initial_snapshot() override;
  void take_new_snapshot() override;
  void calculate() override;  // time_delta_seconds is not strictly needed here
  void commit() override;

  // This internal helper reads the /proc directory and returns the raw snapshot
  // list
  ProcessSnapshotMap read_data();
  void set_process_count(int);
};
using ProcessPollingTaskPtr = std::unique_ptr<ProcessPollingTask>;
