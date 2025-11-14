
#include "load_avg.hpp"

#include <iomanip>
#include <map>
#include <memory>  // For std::unique_ptr
#include <thread>  // For sleep_for
#include <type_traits>
#include <vector>

#include "data.h"
#include "data_local.h"
#include "data_ssh.h"

std::istream& LocalDataStreams::get_loadavg_stream() {
  reset_stream(loadavg, "/proc/loadavg");
  return loadavg;
}
std::istream& ProcDataStreams::get_loadavg_stream() {
  std::string loadavg_data = execute_ssh_command("cat /proc/loadavg");
  loadavg.str(loadavg_data);
  //   rewind(loadavg, "loadavg");
  return loadavg;
}

void get_load_and_process_stats(DataStreamProvider& provider,
                                SystemMetrics& metrics) {
  // 1. Get Load Average
  provider.get_loadavg_stream() >> metrics.load_avg_1m >> metrics.load_avg_5m >>
      metrics.load_avg_15m;

  // 2. Get Process Counts
  std::istream& stat_stream = provider.get_stat_stream();
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
