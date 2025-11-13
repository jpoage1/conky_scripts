// networkstats.hpp
#pragma once

#include <chrono>
#include <istream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class DataStreamProvider;
struct SystemMetrics;
struct NetworkSnapshot;

struct NetworkSnapshot {
  std::string interface_name;
  unsigned long long rx_bytes = 0;
  unsigned long long rx_packets = 0;
  unsigned long long tx_bytes = 0;
  unsigned long long tx_packets = 0;
};

/**
 * @brief Holds the calculated transfer rates for one network interface.
 */
struct NetworkInterfaceStats {
  std::string interface_name;
  double rx_bytes_per_sec = 0.0;
  double tx_bytes_per_sec = 0.0;
  // Add packet rates if needed
};
/**
 * @brief Reads network stats, calculates rates, and populates metrics.
 * Manages internal state (previous snapshot/timestamp).
 * @param provider The data stream provider.
 * @param metrics The SystemMetrics struct to populate.
 * @param prev_snapshot Reference to the map holding the previous snapshot.
 * @param prev_timestamp Reference to the time_point of the previous snapshot.
 * @param initialized Reference to the boolean tracking initialization.
 */
void get_network_stats(DataStreamProvider& provider, SystemMetrics& metrics);

void get_network_stats(DataStreamProvider& provider);
std::map<std::string, NetworkSnapshot> read_network_snapshot(
    std::istream& net_dev_stream);

std::vector<NetworkInterfaceStats> calculate_network_rates(
    const std::map<std::string, NetworkSnapshot>& prev_snapshot,
    const std::map<std::string, NetworkSnapshot>& current_snapshot,
    double time_delta_seconds
);
