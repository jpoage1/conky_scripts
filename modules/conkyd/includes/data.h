#pragma once
#include <cstdint>
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

#include "corestat.h"
#include "networkstats.hpp"
#include "processinfo.hpp"

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
};

class DataStreamProvider {
 public:
  void rewind(std::stringstream& stream, const std::string& streamName) {
    if (stream.fail() || stream.bad()) {
        std::cerr << "DEBUG: Stream '" << streamName
                << "' was in fail/bad state before rewind." << std::endl;
    }

    stream.clear();
    stream.seekg(0, std::ios::beg);

    if (stream.fail() || stream.bad()) {
        std::cerr << "DEBUG: Stream '" << streamName
                << "' is still in fail/bad state after rewind. FATAL."
                << std::endl;
    }
  }

  void rewind(std::ifstream& stream, const std::string& streamName) {
    // Debug: Check if stream is in a bad state before attempting reset
    if (stream.fail() ) {
        std::cerr << "DEBUG: Stream '" << streamName
                << "' was in fail state before rewind." << std::endl;
    } else if ( stream.bad()) {
        std::cerr << "DEBUG: Stream '" << streamName
                << "' was in bad state before rewind." << std::endl;
    } else  {
        std::cerr << "DEBUG: Stream '" << streamName
                << "' was in good state before rewind." << std::endl;
    }

      stream.clear();
      stream.seekg(0, std::ios::beg);

      // Debug: Confirm stream is usable after reset
      if (stream.fail() || stream.bad()) {
        std::cerr << "DEBUG: Stream '" << streamName
                  << "' is still in fail/bad state after rewind. FATAL."
                  << std::endl;
      }
  }
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
public:
    /**
     * @brief Constructs a task by storing references to its context.
     * * We use an initializer list (the ': ...') because references
     * MUST be initialized, they cannot be assigned later.
     */
    IPollingTask(DataStreamProvider& _provider, SystemMetrics& _metrics)
        : provider(_provider),
          metrics(_metrics)
    {
    }
    virtual ~IPollingTask() = default;

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

class CpuPollingTask : public IPollingTask {
private:
    std::vector<CpuSnapshot> t1_snapshots;
    std::vector<CpuSnapshot> t2_snapshots;

public:
    CpuPollingTask(DataStreamProvider& _provider, SystemMetrics& _metrics)
        : IPollingTask(_provider, _metrics) {}
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
        : IPollingTask(_provider, _metrics) {}
    void take_snapshot_1() override {
        t1_snapshot = read_network_snapshot(provider.get_net_dev_stream());
    }

    void take_snapshot_2() override {
        t2_snapshot = read_network_snapshot(provider.get_net_dev_stream());
    }

    void calculate(double time_delta_seconds) override {
        metrics.network_interfaces = calculate_network_rates(
            t1_snapshot,
            t2_snapshot,
            time_delta_seconds // Network *does* use the time_delta
        );
    }
};

std::vector<std::unique_ptr<IPollingTask>> read_data(DataStreamProvider&, SystemMetrics&);

void print_metrics(const SystemMetrics&);
void get_load_and_process_stats(DataStreamProvider& provider,
                                SystemMetrics& metrics);
void get_top_processes_mem(DataStreamProvider& provider,
                           SystemMetrics& metrics);

void get_top_processes_cpu(DataStreamProvider& provider,
                           SystemMetrics& metrics);

void poll_dynamic_stats(DataStreamProvider& provider, SystemMetrics& metrics);
void get_system_info(SystemMetrics &metrics);
