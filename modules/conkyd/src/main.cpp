
#include "data_local.h"
#include "data_ssh.h"
SystemMetrics get_metrics(const bool);
// SystemMetrics get_ssh_metrics();
// SystemMetrics get_local_metrics();

int main() {
  print_metrics(get_metrics(false));
  std::cout << "-------------" << std::endl;
  print_metrics(get_metrics(true));
  return 0;
}

SystemMetrics get_metrics(const bool use_ssh) {
  LocalDataStreams local_streams;
  ProcDataStreams ssh_streams;
  DataStreamProvider *provider = nullptr;
  SystemMetrics metrics;

  if (use_ssh) {
    if (setup_ssh_session() != 0) {
      std::cerr << "Failed to set up SSH session. Exiting." << std::endl;
      return metrics;
    }
    ssh_streams = get_ssh_streams();
    provider = &ssh_streams;
  } else {
    local_streams = get_local_file_streams();
    provider = &local_streams;
  }

  if (provider) {
    metrics = read_data(*provider);
  } else {
    std::cerr << "Error: No data provider could be initialized." << std::endl;
    return metrics;
  }
  if (use_ssh) {
    cleanup_ssh_session();
  }
  return metrics;
}

// SystemMetrics get_local_metrics() {
//   LocalDataStreams streams = get_local_file_streams();

//   SystemMetrics metrics = read_data(streams);
//   return metrics;
// }

// SystemMetrics get_ssh_metrics() {
//   SystemMetrics metrics;

//   if (setup_ssh_session() != 0) {
//     std::cerr << "Failed to set up SSH session." << std::endl;
//     return metrics;
//   }
//   ProcDataStreams streams = get_ssh_streams();

//   metrics = read_data(streams);
//   cleanup_ssh_session();
//   return metrics;
// }
