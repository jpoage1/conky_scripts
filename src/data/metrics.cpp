// metrics.cpp
#include "metrics.hpp"

#include "context.hpp"
#include "cpuinfo.hpp"
#include "data_local.hpp"
#include "data_ssh.hpp"
#include "diskstat.hpp"
#include "filesystems.hpp"
#include "load_avg.hpp"
#include "log.hpp"
#include "networkstats.hpp"
#include "pcn.hpp"
#include "polling.hpp"
#include "processinfo.hpp"
#include "provider.hpp"
#include "ssh.hpp"
#include "stream_provider.hpp"
#include "sysinfo.hpp"
#include "uptime.hpp"

void SystemMetrics::complete() {}

void SystemMetrics::configure_provider(MetricsContext& context) {
  std::unique_ptr<DataStreamProvider> _provider;
  switch (context.provider) {
    case DataStreamProviders::LocalDataStream: {
      _provider = std::make_unique<LocalDataStreams>();
    } break;
    case DataStreamProviders::ProcDataStream: {
      // Check for specific host/user
      if (context.host != "" && context.user != "") {
        _provider =
            std::make_unique<ProcDataStreams>(context.host, context.user);
      } else {
        _provider = std::make_unique<ProcDataStreams>();
      }
    } break;
  }
  provider = std::move(_provider);
}

SystemMetrics::SystemMetrics(MetricsContext& context) {
  configure_provider(context);
  create_pipeline(context);
  configure_polling_pipeline(context);
}
void SystemMetrics::create_pipeline(MetricsContext& context) {
  auto settings = context.settings;

  // Task: Battery Info
  task_pipeline.emplace_back([this, &context]() {
    this->battery_info =
        provider->get_battery_status(context.settings.batteries);
  });

  // Task: CPU Temp
  if (settings.enable_cpu_temp) {
    task_pipeline.emplace_back(
        [this]() { cpu_temp_c = provider->get_cpu_temperature(); });
  }

  // Task: Memory
  if (settings.enable_memory) {
    task_pipeline.emplace_back([this]() {
      get_mem_usage(provider->get_meminfo_stream(), meminfo, swapinfo);
    });
  }

  // Task: Uptime & Freq
  if (settings.enable_uptime) {
    task_pipeline.emplace_back([this]() {
      uptime = get_uptime(provider->get_uptime_stream());
      cpu_frequency_ghz = get_cpu_freq_ghz(provider->get_cpuinfo_stream());
    });
  }

  // Task: Load Avg & Processes
  if (settings.enable_load_and_process_stats) {
    task_pipeline.emplace_back([this]() {
      get_load_and_process_stats(provider->get_loadavg_stream(), *this);
    });
  }

  // Task: System Info
  if (settings.enable_sysinfo) {
    task_pipeline.emplace_back([this]() { get_system_info(*this); });
  }
}

int SystemMetrics::read_data() {
  // The loop is now dumb; it just executes whatever was configured.
  for (const auto& task : task_pipeline) {
    task();
  }
  return 0;
}

void SystemMetrics::configure_polling_pipeline(MetricsContext& context) {
  auto settings = context.settings;
  std::unique_ptr<IPollingTask>* new_task;

  if (settings.enable_cpuinfo) {
    new_task = &polling_tasks.emplace_back(
        std::make_unique<CpuPollingTask>(*provider, *this, context));
    DEBUG_PTR("cpuinfo", new_task);
  }

  if (settings.enable_network_stats) {
    new_task = &polling_tasks.emplace_back(
        std::make_unique<NetworkPollingTask>(*provider, *this, context));
    DEBUG_PTR("networkstats", new_task);
  }
  if (settings.enable_diskstat) {
    new_task = &polling_tasks.emplace_back(
        std::make_unique<DiskPollingTask>(*provider, *this, context));
    DEBUG_PTR("new_task", new_task);
  }
  if (settings.enable_processinfo()) {
    new_task = &polling_tasks.emplace_back(
        std::make_unique<ProcessPollingTask>(*provider, *this, context));
    DEBUG_PTR("processinfo", new_task);
  }
  (void)new_task;
}
