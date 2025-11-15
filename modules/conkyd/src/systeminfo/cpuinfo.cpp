// cpuinfo.cpp
#include "cpuinfo.h"

#include <fstream>
#include <string>

#include "data_local.h"
#include "data_ssh.h"

std::istream& LocalDataStreams::get_cpuinfo_stream() {
  reset_stream(cpuinfo, "/proc/cpuinfo");
  return cpuinfo;
}

std::istream& ProcDataStreams::get_cpuinfo_stream() {
  std::string cpu_data = execute_ssh_command("cat /proc/cpuinfo");
  cpuinfo.str(cpu_data);
  rewind(cpuinfo, "cpuinfo");
  return cpuinfo;
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
