#include "metrics.hpp"

#include <iostream>

#include "data_local.h"
#include "data_ssh.h"
#include "diskstat.h"

// This function's logic is restructured to avoid the null pointer
int get_metrics(const std::string& config_file, const bool use_ssh) {
  SystemMetrics metrics;

  if (use_ssh) {
    if (setup_ssh_session() != 0) {
      std::cerr << "Failed to set up SSH session. Exiting." << std::endl;
      return 2;
    }
    ProcDataStreams ssh_streams = get_ssh_streams();  // <--- Object created

    // Logic is moved inside the 'if' block
    metrics = read_data(ssh_streams);  // <--- No *
    print_metrics(metrics);
    diskstat(ssh_streams, config_file);  // <--- No *

    cleanup_ssh_session();
  } else {
    LocalDataStreams local_streams =
        get_local_file_streams();  // <--- Object created

    // Logic is moved inside the 'else' block
    metrics = read_data(local_streams);  // <--- No *
    print_metrics(metrics);
    diskstat(local_streams, config_file);  // <--- No *
  }

  return 0;
}

/**
 * @brief Gets metrics from the local filesystem.
 */
int get_local_metrics(const std::string& config_file,
                      CombinedMetrics& metrics) {
  LocalDataStreams local_streams = get_local_file_streams();
  // Pass the object directly, not its address
  return get_metrics_from_provider(local_streams, config_file,
                                   metrics);  // <--- Changed
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

  ProcDataStreams ssh_streams = get_ssh_streams();
  // Pass the object directly, not its address
  int result = get_metrics_from_provider(ssh_streams, config_file,
                                         metrics);  // <--- Changed
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

  ProcDataStreams ssh_streams = get_ssh_streams();
  // Pass the object directly, not its address
  int result = get_metrics_from_provider(ssh_streams, config_file,
                                         metrics);  // <--- Changed
  cleanup_ssh_session();
  return result;
}

/**
 * @brief The new core logic function.
 * * This private helper function contains the logic from the old get_metrics
 * function. It is generic and operates on any DataStreamProvider.
 */
// Accepts a reference (&) instead of a pointer (*)
int get_metrics_from_provider(DataStreamProvider& provider,  // <--- Changed
                              const std::string& config_file,
                              CombinedMetrics& metrics) {
  // A reference cannot be null, so the null check is removed.
  // if (!provider) { ... } // <--- Removed

  std::vector<std::string> device_paths;
  if (read_device_paths(config_file, device_paths) == 1) {
    std::cout << "${color red}Unable to load file: " + config_file + "${color}"
              << std::endl;
    return 1;
  }

  // No dereferencing (*) needed, use object syntax
  metrics.system = read_data(provider);                         // <--- Changed
  metrics.disks = collect_device_info(provider, device_paths);  // <--- Changed
  return 0;
}
