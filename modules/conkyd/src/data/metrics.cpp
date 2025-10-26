#include "metrics.hpp"

#include <iostream>

#include "data_local.h"
#include "data_ssh.h"
#include "diskstat.h"

int get_metrics(const std::string& config_file, const bool use_ssh) {
  LocalDataStreams local_streams;
  ProcDataStreams ssh_streams;
  DataStreamProvider* provider = nullptr;
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

/**
 * @brief Gets metrics from the local filesystem.
 */
int get_local_metrics(const std::string& config_file,
                      CombinedMetrics& metrics) {
  LocalDataStreams local_streams = get_local_file_streams();
  return get_metrics_from_provider(&local_streams, config_file, metrics);
}

/**
 * @brief Gets metrics from the default SSH server.
 */
int get_server_metrics(const std::string& config_file,
                       CombinedMetrics& metrics) {
  if (setup_ssh_session() != 0) {  // Calls the default version
    std::cerr << "Failed to set up default SSH session. Exiting." << std::endl;
    return 2;
  }

  ProcDataStreams ssh_streams = get_ssh_streams();
  int result = get_metrics_from_provider(&ssh_streams, config_file, metrics);
  cleanup_ssh_session();
  return result;
}

/**
 * @brief Overloaded function to get metrics from a specific SSH server.
 */
int get_server_metrics(const std::string& config_file, CombinedMetrics& metrics,
                       const std::string& host, const std::string& user) {
  if (setup_ssh_session(host, user) != 0) {  // Calls the parameterized version
    std::cerr << "Failed to set up SSH session to " << user << "@" << host
              << ". Exiting." << std::endl;
    return 2;
  }

  ProcDataStreams ssh_streams = get_ssh_streams();
  int result = get_metrics_from_provider(&ssh_streams, config_file, metrics);
  cleanup_ssh_session();
  return result;
}

/**
 * @brief The new core logic function.
 * * This private helper function contains the logic from the old get_metrics
 * function. It is generic and operates on any DataStreamProvider.
 */
int get_metrics_from_provider(DataStreamProvider* provider,
                              const std::string& config_file,
                              CombinedMetrics& metrics) {
  if (!provider) {
    std::cerr << "Error: No data provider could be initialized." << std::endl;
    return 1;
  }

  std::vector<std::string> device_paths;
  if (read_device_paths(config_file, device_paths) == 1) {
    std::cout << "${color red}Unable to load file: " + config_file + "${color}"
              << std::endl;
    return 1;
  }

  metrics.system = read_data(*provider);
  metrics.disks = collect_device_info(*provider, device_paths);
  return 0;
}
