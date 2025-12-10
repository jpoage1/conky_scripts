// metrics.hpp
#pragma once

#include "corestat.hpp"
#include "data.hpp"
#include "diskstat.hpp"
#include "meminfo.hpp"
#include "networkstats.hpp"
#include "pcn.hpp"
#include "provider.hpp"

class IPollingTask;
struct DeviceInfo;
struct ProcessInfo;
struct DiskUsage;
struct LocalDataStreams;
struct ProcDataStreams;
struct MetricsContext;

using PollingTaskList = std::vector<std::unique_ptr<IPollingTask>>;
using DevicePaths = std::vector<std::string>;
struct SystemMetrics {
  std::unique_ptr<DataStreamProvider> provider;
  PollingTaskList polling_tasks;
  std::vector<DeviceInfo> disks;
  std::map<std::string, HdIoStats> disk_io;
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

  SystemMetrics();
  SystemMetrics(MetricsContext& context);

  int read_data();
  void complete();
  DevicePaths load_device_paths(const std::string& config_file);
  int get_metrics_from_provider();
  SystemMetrics(SystemMetrics&&) noexcept = default;
  SystemMetrics& operator=(SystemMetrics&&) noexcept = default;

  SystemMetrics(const SystemMetrics&) = delete;
  SystemMetrics& operator=(const SystemMetrics&) = delete;
};
// int get_local_metrics(DataStreamProvider& provider,
//                       const std::string& config_file, SystemMetrics&
//                       metrics);
// int get_metrics_from_provider(DataStreamProvider& provider,
//                               const std::string& config_file,
//                               SystemMetrics& metrics);

// int get_server_metrics(DataStreamProvider& provider,
//                        const std::string& config_file,
//                        SystemMetrics& metrics);

// int get_server_metrics(DataStreamProvider& provider,
//                        const std::string& config_file, SystemMetrics&
//                        metrics, const std::string& host, const std::string&
//                        user);

int get_local_metrics(DataStreamProviderPtr&, const std::string& config_file,
                      SystemMetrics& metrics);
int get_metrics_from_provider(DataStreamProviderPtr&,
                              const std::string& config_file,
                              SystemMetrics& metrics);
int get_server_metrics(DataStreamProviderPtr&, const std::string& config_file,
                       SystemMetrics& metrics);

int get_server_metrics(DataStreamProviderPtr&, const std::string& config_file,
                       SystemMetrics& metrics, const std::string& host,
                       const std::string& user);
