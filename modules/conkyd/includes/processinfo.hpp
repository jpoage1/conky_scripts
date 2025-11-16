// processinfo.hpp
#pragma once

#include "data_local.h"
#include "data_ssh.h"
#include "pcn.hpp"

enum class ProcessParseType { TopCPU, TopMem };

// Data structure to hold information about a single process
struct ProcessInfo {
  int pid = 0;
  long vmRssKb = 0;  // Resident Set Size in KiB
  double cpu_percent = 0.0;
  double mem_percent = 0.0;
  std::string name;
};
using ParseStrategy = std::function<bool(std::stringstream&, ProcessInfo&)>;

void get_top_mem_processes(std::istream& stream, SystemMetrics& metrics);
void get_top_cpu_processes(std::istream& stream, SystemMetrics& metrics);
void get_top_processes(std::istream& stream,
                       std::vector<ProcessInfo>& output_list, long mem_total_kb,
                       ProcessParseType type);
