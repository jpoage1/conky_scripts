// mem_processes.cpp
#include "mem_processes.hpp"

std::istream& LocalDataStreams::get_top_mem_processes_stream() {
  // Command: Get PID, RSS (in KiB), and command name.
  // --no-headers simplifies parsing.
  // --sort=-rss sorts by RSS descending.
  // head -n 10 gets the top 10.
  const char* cmd =
      "ps -eo pid,rss,comm --no-headers --sort=-rss | grep -v \" ps$\" | head "
      "-n 10";

  std::string cmd_output = exec_local_cmd(cmd);

  top_mem_procs.str(std::move(cmd_output));  // Move output into stream
  rewind(top_mem_procs, "top_mem_procs");
  return top_mem_procs;
}
std::istream& ProcDataStreams::get_top_mem_processes_stream() {
  std::string top_mem_data = execute_ssh_command(
      "ps -eo pid,rss,comm --no-headers --sort=-rss | grep -v \" ps$\" | "
      "head "
      "-n 10");
  top_mem_procs.str(top_mem_data);
  rewind(top_mem_procs, "top_mem_procs");
  return top_mem_procs;
};

void get_top_processes_mem(std::istream& stream, SystemMetrics& metrics) {
  metrics.top_processes_mem.clear();  // Clear old data
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
