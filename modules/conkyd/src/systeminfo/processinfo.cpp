// processinfo.cpp
#include "processinfo.hpp"
namespace {

// Command: Get PID, RSS (in KiB), and command name.
// --no-headers simplifies parsing.
// --sort=-rss sorts by RSS descending.
// head -n 10 gets the top 10.
const char* top_mem_processes_avg_cmd =
    "ps -eo pid,%cpu,rss,comm --no-headers --sort=-rss | grep -v \" ps$\" | "
    "head "
    "-n 10";
const char* top_cpu_processes_avg_cmd =
    "ps -eo pid,%cpu,rss,comm --no-headers --sort=-%cpu | grep -v \" ps$\" | "
    "head -n 10";
const char* top_mem_processes_real_cmd =
    "top -b -n 2 -d 0.9 -o %CPU | "  // 2 iterations, 0.9s apart, sorted by CPU
    "awk 'BEGIN{RS=\"\"} NR==2' | "  // Get only the second iteration's data
    "tail -n +8 | "                  // Skip the 7 header lines
    "head -n 10 | "                  // Get the top 10 processes
    "awk '{print $1, $9, $6, $12}'";
const char* top_mem_processes_real_cmd =
    "top -b -n 2 -d 0.9 -o %MEM | "  // <-- FIX: Changed -o %CPU to -o %MEM
    "awk 'BEGIN{RS=\"\"} NR==2' | "
    "tail -n +8 | "
    "head -n 10 | "
    "awk '{print $1, $9, $6, $12}'";
};  // namespace
std::istream& LocalDataStreams::get_top_mem_processes_avg_stream() {
  return create_stream_from_command(top_mem_procs, top_mem_processes_avg_cmd);
}
std::istream& ProcDataStreams::get_top_mem_processes_avg_stream() {
  return create_stream_from_command(top_mem_procs, top_mem_processes_avg_cmd);
};
std::istream& LocalDataStreams::get_top_cpu_processes_avg_stream() {
  return create_stream_from_command(top_mem_procs, top_cpu_processes_avg_cmd);
}
std::istream& ProcDataStreams::get_top_cpu_processes_avg_stream() {
  return create_stream_from_command(top_mem_procs, top_cpu_processes_avg_cmd);
}

/* Real Time */

std::istream& LocalDataStreams::get_top_mem_processes_real_stream() {
  return create_stream_from_command(top_mem_procs, top_mem_processes_real_cmd);
}
std::istream& ProcDataStreams::get_top_mem_processes_real_stream() {
  return create_stream_from_command(top_mem_procs, top_mem_processes_real_cmd);
};
std::istream& LocalDataStreams::get_top_cpu_processes_real_stream() {
  return create_stream_from_command(top_mem_procs, top_cpu_processes_real_cmd);
}
std::istream& ProcDataStreams::get_top_cpu_processes_real_stream() {
  return create_stream_from_command(top_mem_procs, top_cpu_processes_real_cmd);
}

void get_top_processes(std::istream& stream,
                       std::vector<ProcessInfo>& output_list,
                       long mem_total_kb) {
  output_list.clear();  // Clear the destination vector
  std::string line;
  double total_mem_kb =
      (mem_total_kb > 0) ? static_cast<double>(mem_total_kb) : 1.0;

  while (std::getline(stream, line)) {
    if (line.empty()) continue;

    std::stringstream ss(line);
    ProcessInfo proc;

    if (ss >> proc.pid >> proc.cpu_percent >> proc.vmRssKb) {
      //  Perform common calculations and parsing
      proc.mem_percent =
          (static_cast<double>(proc.vmRssKb) / total_mem_kb) * 100.0;

      // The rest of the line is the command name
      std::getline(ss, proc.name);

      // trim leading whitespace
      size_t first = proc.name.find_first_not_of(" \t");
      if (std::string::npos != first) {
        proc.name = proc.name.substr(first);
      } else {
        proc.name = "unknown";
      }

      output_list.push_back(proc);
    }
  }
}
