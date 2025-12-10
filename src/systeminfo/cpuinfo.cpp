// cpuinfo.cpp
#include "cpuinfo.hpp"

#include "data_local.hpp"
#include "data_ssh.hpp"

std::istream& LocalDataStreams::get_cpuinfo_stream() {
  return create_stream_from_file(cpuinfo, "/proc/cpuinfo");
}

std::istream& ProcDataStreams::get_cpuinfo_stream() {
  return create_stream_from_command(cpuinfo, "cat /proc/cpuinfo");
}

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
