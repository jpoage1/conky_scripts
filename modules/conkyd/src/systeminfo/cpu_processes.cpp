// cpu_processes.cpp
#include "cpu_processes.hpp"

std::istream& LocalDataStreams::get_top_cpu_processes_stream() {
  // Command sorted by %cpu
  const char* cmd =
      "ps -eo pid,%cpu,rss,comm --no-headers --sort=-%cpu | grep -v \" ps$\" | "
      "head -n 10";
  std::string cmd_output = exec_local_cmd(cmd);

  top_cpu_procs.str(std::move(cmd_output));  // Use the cpu stream member
  rewind(top_cpu_procs, "top_cpu_procs");
  return top_cpu_procs;
}

std::istream& ProcDataStreams::get_top_cpu_processes_stream() {
  std::string top_cpu_data = execute_ssh_command(
      "ps -eo pid,%cpu,rss,comm --no-headers --sort=-%cpu | grep -v \" ps$\" "
      "| "
      "head -n 10");

  top_cpu_procs.str(top_cpu_data);
  //   rewind(top_cpu_procs, "top_cpu_procs");
  return top_cpu_procs;
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
