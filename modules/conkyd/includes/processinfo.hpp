// processinfo.hpp
#pragma once

#include "data_local.h"
#include "data_ssh.h"
#include "pcn.hpp"

// Data structure to hold information about a single process
struct ProcessInfo {
  int pid = 0;
  long vmRssKb = 0;  // Resident Set Size in KiB
  double cpu_percent = 0.0;
  double mem_percent = 0.0;
  std::string name;
};

void get_top_processes(std::istream& stream,
                       std::vector<ProcessInfo>& output_list,
                       long mem_total_kb);
