#include "metrics.hpp"

#include <iostream>

#include "data_local.h"
#include "data_ssh.h"
#include "diskstat.h"

int get_metrics(const std::string &config_file, const bool use_ssh) {
  LocalDataStreams local_streams;
  ProcDataStreams ssh_streams;
  DataStreamProvider *provider = nullptr;
  SystemMetrics metrics;

  if (use_ssh) {
    if (setup_ssh_session() != 0) {
      std::cerr << "Failed to set up SSH session. Exiting." << std::endl;
      return 2;
    }
    ssh_streams = get_ssh_streams();
    provider = &ssh_streams;
  } else {
    local_streams = get_local_file_streams();
    provider = &local_streams;
  }

  if (provider) {
    metrics = read_data(*provider);
    print_metrics(metrics);
    diskstat(*provider, config_file);
  } else {
    std::cerr << "Error: No data provider could be initialized." << std::endl;
    return 1;
  }
  if (use_ssh) {
    cleanup_ssh_session();
  }
  return 0;
}
