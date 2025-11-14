#pragma once
#include <cstdint>
#include <fstream>
#include <iostream>
#include <istream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "corestat.h"
#include "diskstat.hpp"
#include "networkstats.hpp"
#include "processinfo.hpp"
#include "types.h"

struct SystemMetrics {
  std::vector<CoreStats> cores;
  double cpu_frequency_ghz;
  double cpu_temp_c;
  long mem_used_kb;
  long mem_total_kb;
  int mem_percent;
  long swap_used_kb;
  long swap_total_kb;
  int swap_percent;
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
  std::vector<ProcessInfo> top_processes_mem;
  std::vector<ProcessInfo> top_processes_cpu;

  std::vector<DiskIoStats> disk_io_rates;
};

class DataStreamProvider {
 public:
  void rewind(std::stringstream& stream, const std::string& streamName);
  void rewind(std::stringstream& stream);

  void rewind(std::ifstream& stream, const std::string& streamName);
  virtual ~DataStreamProvider() = default;
  virtual std::istream& get_cpuinfo_stream() = 0;
  virtual std::istream& get_meminfo_stream() = 0;
  virtual std::istream& get_uptime_stream() = 0;
  virtual std::istream& get_stat_stream() = 0;
  virtual std::istream& get_mounts_stream() = 0;
  virtual std::istream& get_diskstats_stream() = 0;
  virtual std::istream& get_loadavg_stream() = 0;
  virtual std::istream& get_net_dev_stream() = 0;
  virtual std::istream& get_top_mem_processes_stream() = 0;
  virtual std::istream& get_top_cpu_processes_stream() = 0;
  virtual uint64_t get_used_space_bytes(const std::string& mount_point) = 0;
  virtual uint64_t get_disk_size_bytes(const std::string& mount_point) = 0;
  virtual double get_cpu_temperature() = 0;
};

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

struct CombinedMetrics {
  std::vector<std::unique_ptr<IPollingTask>> polled;
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
  std::vector<CpuSnapshot> t1_snapshots;
  std::vector<CpuSnapshot> t2_snapshots;

 public:
  CpuPollingTask(DataStreamProvider& _provider, SystemMetrics& _metrics)
      : IPollingTask(_provider, _metrics) {
    name = "CPU polling";
  }
  void take_snapshot_1() override {
    t1_snapshots = read_cpu_snapshots(provider.get_stat_stream());
  }

  void take_snapshot_2() override {
    t2_snapshots = read_cpu_snapshots(provider.get_stat_stream());
  }

  void calculate(double /*time_delta_seconds*/) override {
    // CPU usage calc doesn't need the time_delta, but others might
    metrics.cores = calculate_cpu_usages(t1_snapshots, t2_snapshots);
  }
};

class NetworkPollingTask : public IPollingTask {
 private:
  std::map<std::string, NetworkSnapshot> t1_snapshot;
  std::map<std::string, NetworkSnapshot> t2_snapshot;

 public:
  NetworkPollingTask(DataStreamProvider& _provider, SystemMetrics& _metrics)
      : IPollingTask(_provider, _metrics) {
    name = "Network polling";
  }
  void take_snapshot_1() override {
    t1_snapshot = read_network_snapshot(provider.get_net_dev_stream());
  }

  void take_snapshot_2() override {
    t2_snapshot = read_network_snapshot(provider.get_net_dev_stream());
  }

  void calculate(double time_delta_seconds) override {
    metrics.network_interfaces = calculate_network_rates(
        t1_snapshot, t2_snapshot,
        time_delta_seconds  // Network *does* use the time_delta
    );
  }
};
class DiskPollingTask : public IPollingTask {
 private:
  std::map<std::string, DiskIoSnapshot> t1_snapshots;
  std::map<std::string, DiskIoSnapshot> t2_snapshots;

 public:
  DiskPollingTask(DataStreamProvider& _provider, SystemMetrics& _metrics)
      : IPollingTask(_provider, _metrics) {
    name = "Disk polling";
  }

  PollingTaskList read_data(DataStreamProvider&, SystemMetrics&);
  void take_snapshot_1() override {
    t1_snapshots = read_disk_io_snapshots(provider.get_diskstats_stream());
  }
  void take_snapshot_2() override {
    t2_snapshots = read_disk_io_snapshots(provider.get_diskstats_stream());
  }

  void calculate(double time_delta_seconds) override {
    // Clear old rates and fill with new ones
    metrics.disk_io_rates.clear();

    if (time_delta_seconds <= 0) return;

    for (auto const& [dev_name, t2_snap] : t2_snapshots) {
      auto t1_it = t1_snapshots.find(dev_name);
      if (t1_it != t1_snapshots.end()) {
        const auto& t1_snap = t1_it->second;

        // Calculate deltas
        uint64_t read_delta = (t2_snap.bytes_read >= t1_snap.bytes_read)
                                  ? (t2_snap.bytes_read - t1_snap.bytes_read)
                                  : 0;

        uint64_t write_delta =
            (t2_snap.bytes_written >= t1_snap.bytes_written)
                ? (t2_snap.bytes_written - t1_snap.bytes_written)
                : 0;

        // Add the calculated rate to our metrics
        metrics.disk_io_rates.push_back(
            {.device_name = dev_name,
             .read_bytes_per_sec =
                 static_cast<uint64_t>(read_delta / time_delta_seconds),
             .write_bytes_per_sec =
                 static_cast<uint64_t>(write_delta / time_delta_seconds)});
      }
    }
  }
};
void read_data(DataStreamProvider&, SystemMetrics&, PollingTaskList&);

PollingTaskList read_data(DataStreamProvider&, SystemMetrics&);

void get_system_info(SystemMetrics& metrics);

void print_system_metrics(const SystemMetrics& metrics);
void print_metrics(const CombinedMetrics& metrics);
void print_metrics(const SystemMetrics& metrics);

void print_device_metrics(const std::vector<DeviceInfo>& devices);
