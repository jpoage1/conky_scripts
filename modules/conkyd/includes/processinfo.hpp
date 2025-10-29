#pragma once

#include <string>
#include <vector>

// Data structure to hold information about a single process
struct ProcessInfo {
  int pid = 0;
  long vmRssKb = 0;  // Resident Set Size in KiB
  std::string name;
};
