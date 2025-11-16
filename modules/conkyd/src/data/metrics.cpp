// metrics.cpp
#include "metrics.hpp"

#include "data.h"
#include "data_local.h"
#include "data_ssh.h"
#include "filesystems.hpp"

/**
 * @brief Gets metrics from the local filesystem.
 */
int get_local_metrics(DataStreamProviderPtr& provider,
                      const std::string& config_file,
                      CombinedMetrics& metrics) {
  //   std::cerr << "Getting local metrics" << std::endl;
  return get_metrics_from_provider(provider, config_file, metrics);
}

/**
 * @brief Gets metrics from the default SSH server.
 */
int get_server_metrics(DataStreamProviderPtr& provider,
                       const std::string& config_file,
                       CombinedMetrics& metrics) {
  if (setup_ssh_session() != 0) {
    std::cerr << "Failed to set up default SSH session. Exiting." << std::endl;
    return 2;
  }

  int result = get_metrics_from_provider(provider, config_file, metrics);
  cleanup_ssh_session();
  return result;
}

/**
 * @brief Overloaded function to get metrics from a specific SSH server.
 */
int get_server_metrics(DataStreamProviderPtr& provider,
                       const std::string& config_file, CombinedMetrics& metrics,
                       const std::string& host, const std::string& user) {
  if (setup_ssh_session(host, user) != 0) {
    std::cerr << "Failed to set up SSH session to " << user << "@" << host
              << ". Exiting." << std::endl;
    return 2;
  }

  int result = get_metrics_from_provider(provider, config_file, metrics);
  cleanup_ssh_session();
  return result;
}

/**
 * @brief The new core logic function.
 * * This private helper function contains the logic from the old get_metrics
 * function. It is generic and operates on any DataStreamProvider.
 */
int get_metrics_from_provider(DataStreamProviderPtr& provider,
                              const std::string& config_file,
                              CombinedMetrics& metrics) {
  std::vector<std::string> device_paths;
  if (read_device_paths(config_file, device_paths) == 1) {
    std::cout << "${color red}Unable to load file: " + config_file + "${color}"
              << std::endl;
    return 1;
  }
  if (!provider) {
    std::cerr << "Provider context is empty" << std::endl;
    return 1;
  }

  //   std::cerr << "Getting metrics from provider" << std::endl;
  metrics.polled = read_data(*provider, metrics.system);
  metrics.disks = collect_device_info(*provider, device_paths);
  return 0;
}
