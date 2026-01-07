// include/metrics.hpp
#ifndef METRICS_HPP
#define METRICS_HPP

#include "batteryinfo.hpp"
#include "corestat.hpp"
#include "diskstat.hpp"
#include "meminfo.hpp"
#include "networkstats.hpp"
#include "pcn.hpp"
#include "provider.hpp"
#include "stream_provider.hpp"
#include "system_stability.hpp"
#include "uptime.hpp"

namespace telemetry {

struct DeviceInfo;
struct ProcessInfo;
struct DiskUsage;
struct LocalDataStreams;
struct ProcDataStreams;
struct MetricsContext;
struct SystemStability;
struct NetworkInterfaceStats;

class DataStreamProvider;
class IPollingTask;

using PollingTaskList = std::vector<std::unique_ptr<IPollingTask>>;
using DevicePaths = std::vector<std::string>;
using DataStreamProviderPtr = std::unique_ptr<DataStreamProvider>;

class SystemMetrics {
public:
  std::vector<std::function<void()>> task_pipeline;
  std::unique_ptr<DataStreamProvider> provider;
  PollingTaskList polling_tasks;
  std::vector<DeviceInfo> disks;
  std::map<std::string, HdIoStats> disk_io;
  std::vector<CoreStats> cores;
  double cpu_frequency_ghz;
  double cpu_temp_c;
  MemInfo meminfo;
  MemInfo swapinfo;
  Time uptime;
  std::vector<BatteryStatus> battery_info;
  SystemStability stability;

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
  SystemMetrics(MetricsContext &context);

  int read_data();
  void complete();
  int get_metrics_from_provider();
  SystemMetrics(SystemMetrics &&) noexcept = default;
  SystemMetrics &operator=(SystemMetrics &&) noexcept = default;

  SystemMetrics(const SystemMetrics &) = delete;
  SystemMetrics &operator=(const SystemMetrics &) = delete;

  void configure_polling_pipeline(MetricsContext &context);
  void create_pipeline(MetricsContext &context);
  void configure_provider(MetricsContext &context);
};

int get_local_metrics(DataStreamProviderPtr &, const std::string &config_file,
                      SystemMetrics &metrics);
int get_metrics_from_provider(DataStreamProviderPtr &,
                              const std::string &config_file,
                              SystemMetrics &metrics);
int get_server_metrics(DataStreamProviderPtr &, const std::string &config_file,
                       SystemMetrics &metrics);

int get_server_metrics(DataStreamProviderPtr &, const std::string &config_file,
                       SystemMetrics &metrics, const std::string &host,
                       const std::string &user);
}; // namespace telemetry
#endif
