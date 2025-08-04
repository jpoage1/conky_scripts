#include "cpuinfo.h"

#include <fstream>
#include <string>

float get_cpu_freq_mhz() {
  std::ifstream file("/proc/cpuinfo");
  std::string line;
  while (std::getline(file, line)) {
    if (line.find("cpu MHz") != std::string::npos) {
      return std::stof(line.substr(line.find(":") + 1));
    }
  }
  return 0.0f;
}

float get_cpu_freq_ghz() { return get_cpu_freq_mhz() / 1000.0f; }
