#include "metrics.hpp"

#include <iostream>
#include <memory>

#include "data_local.h"
#include "data_ssh.h"
#include "diskstat.h"
#include "data.h"

/**
 * @brief Gets metrics from the local filesystem.
 */
int get_local_metrics(const std::string& config_file,
                      CombinedMetrics& metrics) {
  LocalDataStreams local_streams;

  return get_metrics_from_provider(local_streams, config_file,
                                   metrics);
}

/**
 * @brief Gets metrics from the default SSH server.
 */
int get_server_metrics(const std::string& config_file,
                       CombinedMetrics& metrics) {
  if (setup_ssh_session() != 0) {
    std::cerr << "Failed to set up default SSH session. Exiting." << std::endl;
    return 2;
  }

  ProcDataStreams ssh_streams;

  int result = get_metrics_from_provider(ssh_streams, config_file,
                                         metrics);
  cleanup_ssh_session();
  return result;
}

/**
 * @brief Overloaded function to get metrics from a specific SSH server.
 */
int get_server_metrics(const std::string& config_file, CombinedMetrics& metrics,
                       const std::string& host, const std::string& user) {
  if (setup_ssh_session(host, user) != 0) {
    std::cerr << "Failed to set up SSH session to " << user << "@" << host
              << ". Exiting." << std::endl;
    return 2;
  }

  ProcDataStreams ssh_streams;

  int result = get_metrics_from_provider(ssh_streams, config_file,
                                         metrics);
  cleanup_ssh_session();
  return result;
}

/**
 * @brief The new core logic function.
 * * This private helper function contains the logic from the old get_metrics
 * function. It is generic and operates on any DataStreamProvider.
 */
int get_metrics_from_provider(DataStreamProvider& provider,
                              const std::string& config_file,
                              CombinedMetrics& metrics) {

  std::vector<std::string> device_paths;
  if (read_device_paths(config_file, device_paths) == 1) {
    std::cout << "${color red}Unable to load file: " + config_file + "${color}"
              << std::endl;
    return 1;
  }

  metrics.polled = read_data(provider, metrics.system);
  metrics.disks = collect_device_info(provider, device_paths);
  return 0;
}
