#include <istream>

// #include "data.h"
#include "data_local.h"
#include "data_ssh.h"

SystemMetrics get_ssh_metrics();
SystemMetrics get_local_metrics();

int main() {
  print_metrics(get_local_metrics());
  std::cout << "-------------" << std::endl;
  print_metrics(get_ssh_metrics());
  return 0;
}

SystemMetrics get_local_metrics() {
  LocalDataStreams streams = get_local_file_streams();

  SystemMetrics metrics = read_data(streams);
  return metrics;
}

SystemMetrics get_ssh_metrics() {
  SystemMetrics metrics;

  if (setup_ssh_session() != 0) {
    std::cerr << "Failed to set up SSH session." << std::endl;
    return metrics;
  }
  ProcDataStreams streams = get_ssh_streams();

  metrics = read_data(streams);
  cleanup_ssh_session();
  return metrics;
}
