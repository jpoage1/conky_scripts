#include "data.h"

#include <sys/utsname.h>

#include <chrono>
#include <iomanip>
#include <map>
#include <thread> // For sleep_for
#include <chrono> // For seconds
#include <type_traits>

#include <vector>
#include <memory> // For std::unique_ptr

#include "corestat.h"
#include "cpuinfo.h"
#include "diskstat.h"
#include "meminfo.h"
#include "networkstats.hpp"
#include "processinfo.hpp"
#include "swapinfo.h"
#include "uptime.h"
#include "metrics.hpp"

#include "parser.hpp"

PollingTaskList read_data(DataStreamProvider& provider, SystemMetrics &metrics) {

    PollingTaskList polling_tasks;
    polling_tasks.push_back(std::make_unique<CpuPollingTask>(provider, metrics));
    polling_tasks.push_back(std::make_unique<NetworkPollingTask>(provider, metrics));
    // polling_tasks.push_back(std::make_unique<DiskPollingTask>(provider, metrics));


  metrics.cpu_temp_c = provider.get_cpu_temperature();

  get_mem_usage(provider.get_meminfo_stream(), metrics.mem_used_kb,
                metrics.mem_total_kb, metrics.mem_percent);
  get_swap_usage(provider.get_meminfo_stream(), metrics.swap_used_kb,
                 metrics.swap_total_kb, metrics.swap_percent);

  metrics.uptime = get_uptime(provider.get_uptime_stream());
  metrics.cpu_frequency_ghz = get_cpu_freq_ghz(provider.get_cpuinfo_stream());

  get_load_and_process_stats(provider, metrics);
  get_top_processes_mem(provider, metrics);
  get_top_processes_cpu(provider, metrics);

  get_system_info(metrics);
  return polling_tasks;
}

void get_system_info(SystemMetrics &metrics) {
  struct utsname uts_info;
  if (uname(&uts_info) == 0) {  // 0 indicates success
    metrics.sys_name = uts_info.sysname;
    metrics.node_name = uts_info.nodename;
    metrics.kernel_release = uts_info.release;
    metrics.machine_type = uts_info.machine;
  } else {
    // Handle uname error if needed, e.g., set default strings
    metrics.sys_name = "N/A";
    metrics.node_name = "N/A";
    metrics.kernel_release = "N/A";
    metrics.machine_type = "N/A";
  }
}

void print_metrics(const SystemMetrics& metrics) {
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

  std::cout << "Mem: " << metrics.mem_used_kb << " / " << metrics.mem_total_kb
            << " kB (" << metrics.mem_percent << "%)" << std::endl;

  std::cout << "Swap: " << metrics.swap_used_kb << " / "
            << metrics.swap_total_kb << " kB (" << metrics.swap_percent << "%)"
            << std::endl;

  std::cout << "--- Top Processes (Mem) ---" << std::endl;
  std::cout << "PID\tVmRSS (MiB)\tName" << std::endl;
  for (const auto& proc : metrics.top_processes_mem) {
    double vmRssMiB = static_cast<double>(proc.vmRssKb) / 1024.0;
    std::cout << proc.pid << "\t" << std::fixed << std::setprecision(1)
              << vmRssMiB << "\t\t" << proc.name << std::endl;
  }
  std::cout << "---------------------------" << std::endl;
  std::cout << "--- Top Processes (CPU) ---" << std::endl;
  std::cout << "PID\t%CPU\t\tName" << std::endl;
  // Iterate over the new vector, accessing the cpu_percent field
  for (const auto& proc : metrics.top_processes_cpu) {
    std::cout << proc.pid << "\t" << std::fixed << std::setprecision(1)
              << proc.cpu_percent << "%\t\t" << proc.name << std::endl;
  }
  std::cout << "---------------------------" << std::endl;
}
void get_load_and_process_stats(DataStreamProvider& provider,
                                SystemMetrics& metrics) {
  // 1. Get Load Average
  provider.get_loadavg_stream() >> metrics.load_avg_1m >> metrics.load_avg_5m >>
      metrics.load_avg_15m;

  // 2. Get Process Counts
  std::istream& stat_stream = provider.get_stat_stream();
  std::string line;
  while (std::getline(stat_stream, line)) {
    if (line.rfind("processes", 0) == 0) {
      std::stringstream ss(line);
      std::string key;
      ss >> key >> metrics.processes_total;
    } else if (line.rfind("procs_running", 0) == 0) {
      std::stringstream ss(line);
      std::string key;
      ss >> key >> metrics.processes_running;
    } else if (line.rfind("procs_blocked", 0) == 0) {
      // All process lines are together, so we can stop
      break;
    }
  }
}
void get_top_processes_mem(DataStreamProvider& provider,
                           SystemMetrics& metrics) {
  metrics.top_processes_mem.clear();  // Clear old data
  std::istream& stream = provider.get_top_mem_processes_stream();
  std::string line;
  double total_mem_kb = (metrics.mem_total_kb > 0)
                            ? static_cast<double>(metrics.mem_total_kb)
                            : 1.0;

  while (std::getline(stream, line)) {
    if (line.empty()) continue;

    std::stringstream ss(line);
    ProcessInfo proc;

    // Parse PID and RSS
    if (ss >> proc.pid >> proc.vmRssKb) {
      proc.mem_percent =
          (static_cast<double>(proc.vmRssKb) / total_mem_kb) * 100.0;

      // The rest of the line is the command name, which can have spaces
      std::getline(ss, proc.name);

      // trim leading whitespace from name
      size_t first = proc.name.find_first_not_of(" \t");
      if (std::string::npos != first) {
        proc.name = proc.name.substr(first);
      } else {
        proc.name = "unknown";  // Fallback
      }
      metrics.top_processes_mem.push_back(proc);
    }
  }
}
void get_top_processes_cpu(DataStreamProvider& provider,
                           SystemMetrics& metrics) {
  metrics.top_processes_cpu.clear();  // Clear old data
  std::istream& stream = provider.get_top_cpu_processes_stream();
  std::string line;
  double total_mem_kb = (metrics.mem_total_kb > 0)
                            ? static_cast<double>(metrics.mem_total_kb)
                            : 1.0;
  while (std::getline(stream, line)) {
    if (line.empty()) continue;
    std::stringstream ss(line);
    ProcessInfo proc;  // Use the *same* struct. vmRssKb defaults to 0.

    // Parse PID and %CPU (double). vmRssKb is untouched.
    if (ss >> proc.pid >> proc.cpu_percent >> proc.vmRssKb) {
      proc.mem_percent =
          (static_cast<double>(proc.vmRssKb) / total_mem_kb) * 100.0;

      std::getline(ss, proc.name);

      size_t first = proc.name.find_first_not_of(" \t");
      if (std::string::npos != first) {
        proc.name = proc.name.substr(first);
      } else {
        proc.name = "unknown";
      }
      metrics.top_processes_cpu.push_back(proc);
    }
  }
}
