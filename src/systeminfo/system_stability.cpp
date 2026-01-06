// system_stability.cpp

#include "polling.hpp"

double parse_avg10(const std::string &line) {
  // Look for "avg10="
  size_t pos = line.find("avg10=");
  if (pos == std::string::npos)
    return 0.0;

  // Move index to the start of the numeric value
  pos += 6;

  // Find the space or end of string after the number
  size_t end = line.find(' ', pos);
  try {
    return std::stod(line.substr(pos, end - pos));
  } catch (...) {
    return 0.0;
  }
}

SystemStabilityPollingTask::SystemStabilityPollingTask(DataStreamProvider &p,
                                                       SystemMetrics &m,
                                                       MetricsContext &ctx)
    : IPollingTask(p, m, ctx) {
  name = "System Stability";
}

void SystemStabilityPollingTask::configure() {}
void SystemStabilityPollingTask::take_initial_snapshot() {
  take_new_snapshot();
}
void SystemStabilityPollingTask::take_new_snapshot() {
  // 1. Monitor File Descriptors
  std::ifstream file_nr("/proc/sys/fs/file-nr");
  if (file_nr) {
    file_nr >> metrics.stability.file_descriptors_allocated;
    long unused;
    file_nr >> unused >> metrics.stability.file_descriptors_max;
  }

  // 2. Monitor Memory Pressure (PSI)
  // Format: some avg10=0.00 avg60=0.00 avg300=0.00 total=0
  std::ifstream psi_mem("/proc/pressure/memory");
  std::string label, avg10;
  if (psi_mem >> label >> avg10) {
    // Parse "avg10=0.00"
    size_t pos = avg10.find('=');
    if (pos != std::string::npos) {
      metrics.stability.memory_pressure_some = std::stod(avg10.substr(pos + 1));
    }
  }

  auto parse_psi = [](const std::string &path, double &some, double &full) {
    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line)) {
      if (line.rfind("some", 0) == 0)
        some = parse_avg10(line);
      else if (line.rfind("full", 0) == 0)
        full = parse_avg10(line);
    }
  };
  parse_psi("/proc/pressure/memory", metrics.stability.memory_pressure_some,
            metrics.stability.memory_pressure_full);
  parse_psi("/proc/pressure/io", metrics.stability.io_pressure_some,
            metrics.stability.io_pressure_full);
}
void SystemStabilityPollingTask::calculate() {
} // Direct read is sufficient here
void SystemStabilityPollingTask::commit() {}
