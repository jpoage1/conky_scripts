// metrics.cpp
#include "metrics.hpp"

#include "context.hpp"
#include "cpuinfo.hpp"
#include "data_local.hpp"
#include "data_ssh.hpp"
#include "diskstat.hpp"
#include "filesystems.hpp"
#include "frag_stats.hpp"
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
#include <stdexcept>
namespace telemetry {
// Satisfies the linker, prevents accidental usage
SystemMetrics::SystemMetrics() {
  throw std::runtime_error("SystemMetrics default constructor called: This "
                           "object requires a MetricsContext.");
}

void SystemMetrics::complete() {}

void SystemMetrics::configure_provider(MetricsContext &context) {
  std::unique_ptr<DataStreamProvider> _provider;
  switch (context.provider) {
  case DataStreamProviders::LocalDataStream: {
    _provider = std::make_unique<LocalDataStreams>();
  } break;
  case DataStreamProviders::ProcDataStream: {
    // Check for specific host/user
    if (context.host != "" && context.user != "") {
      _provider = std::make_unique<ProcDataStreams>(context.host, context.user);
    } else {
      _provider = std::make_unique<ProcDataStreams>();
    }
  } break;
  }
  provider = std::move(_provider);
}

SystemMetrics::SystemMetrics(MetricsContext &context) {
  configure_provider(context);
  create_pipeline(context);
  configure_polling_pipeline(context);
}
void SystemMetrics::create_pipeline(MetricsContext &context) {
  auto settings = context.settings;

  // Task: Battery Info // fixme
  if (settings.features.enable_battery_info) {
    task_pipeline.emplace_back([this, &context]() {
      this->battery_info =
          provider->get_battery_status(context.settings.batteries);
    });
  }

  // Task: CPU Temp
  if (settings.features.enable_cpu_temp) {
    task_pipeline.emplace_back(
        [this]() { cpu_temp_c = provider->get_cpu_temperature(); });
  }

  // Task: Memory
  if (settings.features.enable_memory) {
    task_pipeline.emplace_back([this]() {
      get_mem_usage(provider->get_meminfo_stream(), meminfo, swapinfo);
    });
  }

  // Task: Uptime & Freq
  if (settings.features.enable_uptime) {
    task_pipeline.emplace_back([this]() {
      uptime = get_uptime(provider->get_uptime_stream());
      cpu_frequency_ghz = get_cpu_freq_ghz(provider->get_cpuinfo_stream());
    });
  }

  // Task: Load Avg & Processes
  if (settings.features.enable_load_and_process_stats) {
    task_pipeline.emplace_back([this]() {
      get_load_and_process_stats(provider->get_loadavg_stream(), *this);
    });
  }

  // Task: System Info
  if (settings.features.enable_sysinfo) {
    task_pipeline.emplace_back([this]() { get_system_info(*this); });
  }
}

int SystemMetrics::read_data() {
  // The loop is now dumb; it just executes whatever was configured.
  for (const auto &task : task_pipeline) {
    task();
  }
  return 0;
}

void SystemMetrics::configure_polling_pipeline(MetricsContext &context) {
  auto settings = context.settings;
  std::unique_ptr<IPollingTask> *new_task;
#define CREATE_POLLING_TASK(TASK_NAME, POLLING_TASK, CONDITION)                \
  do {                                                                         \
    if (CONDITION) {                                                           \
      new_task = &polling_tasks.emplace_back(                                  \
          std::make_unique<POLLING_TASK>(*provider, *this, context));          \
      DEBUG_PTR(TASK_NAME, new_task);                                          \
    }                                                                          \
  } while (0);

  CREATE_POLLING_TASK("cpuinfo", CpuPollingTask,
                      settings.features.enable_cpuinfo);
  CREATE_POLLING_TASK("stability", SystemStabilityPollingTask,
                      settings.features.enable_stability_info);
  CREATE_POLLING_TASK("networkstats", NetworkPollingTask,
                      settings.features.enable_network_stats);
  CREATE_POLLING_TASK("diskstat", DiskPollingTask,
                      settings.features.enable_diskstat);
  CREATE_POLLING_TASK("processinfo", ProcessPollingTask,
                      settings.features.processes.enable_processinfo());
  CREATE_POLLING_TASK("fragmentation", MemoryFragmentationTask,
                      settings.features.processes.enable_processinfo());
  (void)new_task;
}
}; // namespace telemetry
