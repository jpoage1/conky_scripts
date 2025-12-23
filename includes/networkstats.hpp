// networkstats.hpp
#ifndef NETWORKSTATS_HPP
#define NETWORKSTATS_HPP

#include "pcn.hpp"

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
  std::string ip_address;
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

std::string get_ip_address(const std::string& interface_name);

#endif
