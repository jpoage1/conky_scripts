
#include "load_avg.hpp"

#include <iomanip>
#include <map>
#include <memory>  // For std::unique_ptr
#include <thread>  // For sleep_for
#include <type_traits>
#include <vector>

#include "data_local.hpp"
#include "data_ssh.hpp"
#include "metrics.hpp"
#include "stream_provider.hpp"

std::istream& LocalDataStreams::get_loadavg_stream() {
  return create_stream_from_file(loadavg, "/proc/loadavg");
}
std::istream& ProcDataStreams::get_loadavg_stream() {
  return create_stream_from_command(loadavg, "cat /proc/loadavg");
}

void get_load_and_process_stats(std::istream& stat_stream,
                                SystemMetrics& metrics) {
  // 1. Get Load Average
  stat_stream >> metrics.load_avg_1m >> metrics.load_avg_5m >>
      metrics.load_avg_15m;

  stat_stream.clear();
  stat_stream.seekg(0, std::ios::beg);

  // 2. Get Process Counts
  std::string line;
  while (std::getline(stat_stream, line)) {
    if (line.rfind("processes", 0) == 0) {
      std::stringstream ss(line);
      std::string key;
      ss >> key >> metrics.processes_total;
    } else if (line.rfind("procs_running", 0) == 0) {
      std::stringstream ss(line);
      std::string key;
      ss >> key >> metrics.processes_running;
    } else if (line.rfind("procs_blocked", 0) == 0) {
      // All process lines are together, so we can stop
      break;
    }
  }
}
