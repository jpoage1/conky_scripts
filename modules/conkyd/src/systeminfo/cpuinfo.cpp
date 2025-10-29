// cpuinfo.cpp
#include "cpuinfo.h"

#include <fstream>
#include <string>

float get_cpu_freq_mhz(std::istream& input_stream) {
  std::string line;
  while (std::getline(input_stream, line)) {
    if (line.find("cpu MHz") != std::string::npos) {
      return std::stof(line.substr(line.find(":") + 1));
    }
  }
  return 0.0f;
}

float get_cpu_freq_ghz(std::istream& input_stream) {
  return get_cpu_freq_mhz(input_stream) / 1000.0f;
}
