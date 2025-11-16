// data.cpp
#include "data.h"

#include <sys/utsname.h>

#include "corestat.h"
#include "cpuinfo.h"
#include "diskstat.h"
#include "hwmonitor.hpp"
#include "load_avg.hpp"
#include "meminfo.h"
#include "metrics.hpp"
#include "networkstats.hpp"
#include "parser.hpp"
#include "processinfo.hpp"
#include "sysinfo.hpp"
#include "uptime.hpp"

PollingTaskList read_data(DataStreamProvider& provider,
                          SystemMetrics& metrics) {
  PollingTaskList polling_tasks;
  CpuPollingTaskPtr cpu_tasks =
      std::make_unique<CpuPollingTask>(provider, metrics);
  polling_tasks.push_back(std::move(cpu_tasks));

  NetworkPollingTaskPtr network_polling =
      std::make_unique<NetworkPollingTask>(provider, metrics);
  polling_tasks.push_back(std::move(network_polling));

  DiskPollingTaskPtr disk_polling =
      std::make_unique<DiskPollingTask>(provider, metrics);
  polling_tasks.push_back(std::move(disk_polling));

  metrics.cpu_temp_c = provider.get_cpu_temperature();

  get_mem_usage(provider.get_meminfo_stream(), metrics.meminfo,
                metrics.swapinfo);

  metrics.uptime = get_uptime(provider.get_uptime_stream());
  metrics.cpu_frequency_ghz = get_cpu_freq_ghz(provider.get_cpuinfo_stream());

  get_load_and_process_stats(provider.get_loadavg_stream(), metrics);

  // Call for Avg Top CPU processes
  get_top_processes(
      provider.get_top_mem_processes_avg_stream(),  // Input stream
      metrics.top_processes_avg_mem,                // Output vector
      metrics.meminfo.total_kb                      // Total memory
  );
  get_top_processes(
      provider.get_top_cpu_processes_avg_stream(),  // Input stream
      metrics.top_processes_avg_cpu,                // Output vector
      metrics.meminfo.total_kb                      // Total memory
  );
  //   // Call for Real Top CPU processes
  //   get_top_processes(
  //       provider.get_top_mem_processes_real_stream(),  // Input stream
  //       metrics.top_processes_real_mem,                // Output vector
  //       metrics.meminfo.total_kb                           // Total memory
  //   );
  //   get_top_processes(
  //       provider.get_top_cpu_processes_real_stream(),  // Input stream
  //       metrics.top_processes_real_cpu,                // Output vector
  //       metrics.meminfo.total_kb                           // Total memory
  //   );

  get_system_info(metrics);
  return polling_tasks;
}

void dump_fstream(std::istream& stream) {
  std::stringstream buffer;
  buffer << stream.rdbuf();
  std::cerr << "Dumping buffer: " << std::endl;
  std::cerr << buffer.str() << std::endl;
  std::cerr << "Done" << std::endl;
}
IPollingTask::IPollingTask(DataStreamProvider& _provider,
                           SystemMetrics& _metrics)
    : provider(_provider), metrics(_metrics) {
  //   std::cerr << "Parent constructor called" << std::endl;
  //   dump_fstream(provider.get_stat_stream());
  //   std::cerr << "End of Parent constructor call" << std::endl;
}
void log_stream_state(const std::string time, const std::string stream_name,
                      const std::string state) {
  std::cerr << "DEBUG: Stream " << stream_name << "was in " << state
            << " state " << time << " rewind." << std::endl;
}
void log_stream_state(const std::istream& stream, const LogLevel log_level,
                      const std::string time, const std::string stream_name) {
  if (log_level == LogLevel::None) return;
  const std::string streamName =
      stream_name != "" ? "`" + stream_name + "` " : "";
  if (log_level == LogLevel::Debug) {
    std::cerr << "DEBUG STATE for " << streamName
              << ":"
              // good() is true only if no flags are set
              << " good()=" << std::boolalpha
              << stream.good()
              // eof() is true if end-of-file was reached
              << " eof()=" << std::boolalpha
              << stream.eof()
              // fail() is true on formatting errors or if stream isn't open
              << " fail()=" << std::boolalpha
              << stream.fail()
              // bad() is true on unrecoverable read/write errors
              << " bad()=" << std::boolalpha << stream.bad() << std::endl;
    return;
  }
  if (stream.good() && log_level == LogLevel::Notice) {
    log_stream_state(time, streamName, "good");
  } else if (stream.bad() && log_level == LogLevel::Warning) {
    log_stream_state(time, streamName, "bad");
  } else if (stream.eof() && log_level == LogLevel::Notice) {
    log_stream_state(time, streamName, "eof");
  } else if (stream.fail() && log_level == LogLevel::Warning) {
    log_stream_state(time, streamName, "fail");
  }
}
void log_stream_state(std::istream& stream, const LogLevel log_level,
                      const std::string time) {
  const std::string stream_name = "";
  return log_stream_state(stream, log_level, time, stream_name);
}
void DataStreamProvider::rewind(std::istream& stream, std::string stream_name) {
  LogLevel log_level = LogLevel::Debug;
  log_stream_state(stream, log_level, static_cast<std::string>("before"),
                   stream_name);

  stream.clear();
  stream.seekg(0, std::ios::beg);

  log_stream_state(stream, log_level, static_cast<std::string>("after"),
                   stream_name);
}
void DataStreamProvider::rewind(std::istream& stream) { rewind(stream, ""); }

void print_device_metrics(const std::vector<DeviceInfo>& devices) {
  extern std::tuple<std::string, std::function<FuncType>> conky_columns[];
  extern const size_t CONKY_COLUMNS_COUNT;
  std::cout.setf(std::ios::unitbuf);

  print_column_headers(conky_columns, CONKY_COLUMNS_COUNT);
  print_rows(devices, CONKY_COLUMNS_COUNT);
}

void print_metrics(const CombinedMetrics& metrics) {
  print_system_metrics(metrics.system);
  print_device_metrics(metrics.disks);
}
void print_metrics(const SystemMetrics& metrics) {
  print_system_metrics(metrics);
}
void print_system_metrics(const SystemMetrics& metrics) {
  // Set precision for floating point numbers (percentages, temp, freq)
  std::cout << std::fixed << std::setprecision(1);

  std::cout << "CPU Frequency Ghz: " << metrics.cpu_frequency_ghz << std::endl;
  std::cout << "CPU Temp C: " << metrics.cpu_temp_c << " C" << std::endl;

  std::cout << "--- CPU Usage ---" << std::endl;
  // Loop over the vector of CoreStats (which now contains percentages)
  for (const auto& core : metrics.cores) {
    std::cout << "  Core " << std::setw(2) << core.core_id << ": "
              << std::setw(5) << core.total_usage_percent << "% "
              << "(User: " << std::setw(5) << core.user_percent << "%, "
              << "Sys: " << std::setw(5) << core.system_percent << "%, "
              << "IOWait: " << std::setw(5) << core.iowait_percent << "%)"
              << std::endl;
  }
  std::cout << "-----------------" << std::endl;

  std::cout << "Uptime: " << metrics.uptime << std::endl;

  std::cout << "Mem: " << metrics.meminfo.used_kb << " / "
            << metrics.meminfo.total_kb << " kB (" << metrics.meminfo.percent
            << "%)" << std::endl;

  std::cout << "Swap: " << metrics.swapinfo.used_kb << " / "
            << metrics.swapinfo.total_kb << " kB (" << metrics.swapinfo.percent
            << "%)" << std::endl;

  std::cout << "--- Top Processes (Mem) ---" << std::endl;
  std::cout << "PID\tVmRSS (MiB)\tName" << std::endl;
  for (const auto& proc : metrics.top_processes_avg_mem) {
    double vmRssMiB = static_cast<double>(proc.vmRssKb) / 1024.0;
    std::cout << proc.pid << "\t" << std::fixed << std::setprecision(1)
              << vmRssMiB << "\t\t" << proc.name << std::endl;
  }
  std::cout << "---------------------------" << std::endl;
  std::cout << "--- Top Processes (CPU) ---" << std::endl;
  std::cout << "PID\t%CPU\t\tName" << std::endl;
  // Iterate over the new vector, accessing the cpu_percent field
  for (const auto& proc : metrics.top_processes_avg_cpu) {
    std::cout << proc.pid << "\t" << std::fixed << std::setprecision(1)
              << proc.cpu_percent << "%\t\t" << proc.name << std::endl;
  }
  std::cout << "---------------------------" << std::endl;
}
