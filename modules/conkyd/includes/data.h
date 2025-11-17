// data.h
#pragma once
#include "corestat.h"
#include "diskstat.hpp"
#include "format.hpp"
#include "meminfo.h"
#include "networkstats.hpp"
#include "pcn.hpp"

struct ProcessInfo;
struct DiskUsage;

enum LogLevel {
  Debug,
  Warning,
  Notice,
  None,
};
struct SystemMetrics {
  std::vector<CoreStats> cores;
  double cpu_frequency_ghz;
  double cpu_temp_c;
  MemInfo meminfo;
  MemInfo swapinfo;
  std::string uptime;

  double load_avg_1m = 0.0;
  double load_avg_5m = 0.0;
  double load_avg_15m = 0.0;
  long processes_total = 0;
  long processes_running = 0;

  std::string sys_name;
  std::string node_name;
  std::string kernel_release;
  std::string machine_type;

  std::vector<NetworkInterfaceStats> network_interfaces;
  std::vector<ProcessInfo> top_processes_avg_mem;
  std::vector<ProcessInfo> top_processes_avg_cpu;
  std::vector<ProcessInfo> top_processes_real_mem;
  std::vector<ProcessInfo> top_processes_real_cpu;

  std::vector<DiskIoStats> disk_io_rates;
};

class DataStreamProvider {
 public:
  void rewind(std::istream& stream, std::string streamName);
  void rewind(std::istream& stream);

  virtual ~DataStreamProvider() = default;
  virtual std::istream& get_cpuinfo_stream() = 0;
  virtual std::istream& get_meminfo_stream() = 0;
  virtual std::istream& get_uptime_stream() = 0;
  virtual std::istream& get_stat_stream() = 0;
  virtual std::istream& get_mounts_stream() = 0;
  virtual std::istream& get_diskstats_stream() = 0;
  virtual std::istream& get_loadavg_stream() = 0;
  virtual std::istream& get_net_dev_stream() = 0;
  //   virtual std::istream& get_top_mem_processes_stream() = 0;
  //   virtual std::istream& get_top_cpu_processes_stream() = 0;
  virtual std::istream& get_top_mem_processes_avg_stream() = 0;
  virtual std::istream& get_top_cpu_processes_avg_stream() = 0;
  virtual std::istream& get_top_mem_processes_real_stream() = 0;
  virtual std::istream& get_top_cpu_processes_real_stream() = 0;
  virtual DiskUsage get_disk_usage(const std::string&) = 0;
  //   virtual uint64_t get_used_space_bytes(const std::string& mount_point) =
  //   0; virtual uint64_t get_disk_size_bytes(const std::string& mount_point) =
  //   0;
  virtual double get_cpu_temperature() = 0;
};
using DataStreamProviderPtr = std::unique_ptr<DataStreamProvider>;

void dump_fstream(std::istream& stream);
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
};

using PollingTaskList = std::vector<std::unique_ptr<IPollingTask>>;

using CpuSnapshotList = std::vector<CpuSnapshot>;
using NetworkSnapshotMap = std::map<std::string, NetworkSnapshot>;
using DiskIoSnapshotMap = std::map<std::string, DiskIoSnapshot>;

struct CombinedMetrics {
  PollingTaskList polled;
  SystemMetrics system;
  std::vector<DeviceInfo> disks;
  // Default constructor and destructor
  CombinedMetrics() = default;
  ~CombinedMetrics() = default;

  // Enable move operations
  CombinedMetrics(CombinedMetrics&&) = default;
  CombinedMetrics& operator=(CombinedMetrics&&) = default;

  // Explicitly delete copy operations
  CombinedMetrics(const CombinedMetrics&) = delete;
  CombinedMetrics& operator=(const CombinedMetrics&) = delete;
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

  DiskIoSnapshotMap read_data(std::istream&);
};
using DiskPollingTaskPtr = std::unique_ptr<DiskPollingTask>;

void read_data(DataStreamProvider&, SystemMetrics&, PollingTaskList&);

PollingTaskList read_data(DataStreamProvider&, SystemMetrics&);

void log_stream_state(const std::istream& stream, const LogLevel log_level,
                      const std::string time, const std::string stream_name);

void log_stream_state(std::istream& stream, const LogLevel log_level,
                      const std::string time);

void log_stream_state(const std::string time, const std::string stream_name,
                      const std::string state);
