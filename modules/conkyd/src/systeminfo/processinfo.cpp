// processinfo.cpp
#include "processinfo.hpp"

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

std::istream& LocalDataStreams::get_top_cpu_processes_stream() {
  // Command sorted by %cpu
  const char* cmd =
      "ps -eo pid,%cpu,rss,comm --no-headers --sort=-%cpu | grep -v \" ps$\" | "
      "head -n 10";
  std::string cmd_output = exec_local_cmd(cmd);

  top_cpu_procs.str(std::move(cmd_output));
  rewind(top_cpu_procs, "top_cpu_procs");
  return top_cpu_procs;
}

std::istream& ProcDataStreams::get_top_cpu_processes_stream() {
  std::string top_cpu_data = execute_ssh_command(
      "ps -eo pid,%cpu,rss,comm --no-headers --sort=-%cpu | grep -v \" ps$\" "
      "| "
      "head -n 10");

  top_cpu_procs.str(top_cpu_data);
  rewind(top_cpu_procs, "top_cpu_procs");
  return top_cpu_procs;
}

namespace {
/**
 * @brief Strategy for parsing 'ps -eo pid,%cpu,rss,comm'
 */
bool parse_cpu_strategy(std::stringstream& ss, ProcessInfo& proc) {
  // Returns true on successful parsing
  return static_cast<bool>(ss >> proc.pid >> proc.cpu_percent >> proc.vmRssKb);
}

/**
 * @brief Strategy for parsing 'ps -eo pid,rss,comm'
 */
bool parse_mem_strategy(std::stringstream& ss, ProcessInfo& proc) {
  // Returns true on successful parsing
  return static_cast<bool>(ss >> proc.pid >> proc.vmRssKb);
}
}  // namespace
void get_top_processes(std::istream& stream,
                       std::vector<ProcessInfo>& output_list, long mem_total_kb,
                       ProcessParseType type) {
  output_list.clear();  // Clear the destination vector
  std::string line;
  double total_mem_kb =
      (mem_total_kb > 0) ? static_cast<double>(mem_total_kb) : 1.0;

  // 1. Select the correct parsing strategy
  ParseStrategy parser;
  switch (type) {
    case ProcessParseType::TopCPU:
      parser = parse_cpu_strategy;
      break;
    case ProcessParseType::TopMem:
      parser = parse_mem_strategy;
      break;
  }

  while (std::getline(stream, line)) {
    if (line.empty()) continue;

    std::stringstream ss(line);
    ProcessInfo proc;  // Create a new proc for this line

    // 2. Execute the parser.
    //    parser() will modify ss and proc directly (by reference).
    if (parser(ss, proc)) {
      // 3. Perform common calculations and parsing
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

      // 4. Add the completed proc to the correct output list
      output_list.push_back(proc);
    }
  }
}
// void get_top_mem_processes(std::istream& stream, SystemMetrics& metrics) {
//   metrics.top_processes_mem.clear();  // Clear old data
//   std::string line;
//   double total_mem_kb = (metrics.mem_total_kb > 0)
//                             ? static_cast<double>(metrics.mem_total_kb)
//                             : 1.0;

//   while (std::getline(stream, line)) {
//     if (line.empty()) continue;

//     std::stringstream ss(line);
//     ProcessInfo proc;

//     // Parse PID and RSS
//     if (cb()) {
//       proc.mem_percent =
//           (static_cast<double>(proc.vmRssKb) / total_mem_kb) * 100.0;

//       // The rest of the line is the command name, which can have spaces
//       std::getline(ss, proc.name);

//       // trim leading whitespace from name
//       size_t first = proc.name.find_first_not_of(" \t");
//       if (std::string::npos != first) {
//         proc.name = proc.name.substr(first);
//       } else {
//         proc.name = "unknown";  // Fallback
//       }
//       metrics.top_processes_mem.push_back(proc);
//     }
//   }
// }

// void get_top_cpu_processes(std::istream& stream, SystemMetrics& metrics) {
//   metrics.top_processes_cpu.clear();  // Clears the CPU list
//   std::string line;
//   double total_mem_kb = (metrics.mem_total_kb > 0)
//                             ? static_cast<double>(metrics.mem_total_kb)
//                             : 1.0;

//   while (std::getline(stream, line)) {
//     if (line.empty()) continue;

//     std::stringstream ss(line);
//     ProcessInfo proc;

//     // Parse PID, %CPU, and RSS (CPU format)
//     // ps -eo pid,%cpu,rss,comm
//     if (ss >> proc.pid >> proc.cpu_percent >> proc.vmRssKb) {
//       // Calculate mem_percent as well, since we have the data
//       proc.mem_percent =
//           (static_cast<double>(proc.vmRssKb) / total_mem_kb) * 100.0;

//       std::getline(ss, proc.name);

//       // trim leading whitespace
//       size_t first = proc.name.find_first_not_of(" \t");
//       if (std::string::npos != first) {
//         proc.name = proc.name.substr(first);
//       } else {
//         proc.name = "unknown";
//       }

//       metrics.top_processes_cpu.push_back(proc);  // Pushes to CPU list
//     }
//   }
// }
