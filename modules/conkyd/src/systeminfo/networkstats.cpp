// networkstats.cpp
#include "networkstats.hpp"

#include <chrono>
#include <map>
#include <thread>

#include "data.h"
std::map<std::string, NetworkSnapshot> read_network_snapshot(
    std::istream& net_dev_stream) {
  std::map<std::string, NetworkSnapshot> snapshots;
  std::string line;

  // Skip the first two header lines
  std::getline(net_dev_stream, line);
  std::getline(net_dev_stream, line);

  while (std::getline(net_dev_stream, line)) {
    std::stringstream ss(line);
    std::string interface_part;
    ss >> interface_part;  // e.g., "eth0:" or " face |bytes    packets errs
                           // drop fifo frame compressed multicast|bytes ..."

    // Remove trailing colon if present
    if (!interface_part.empty() && interface_part.back() == ':') {
      interface_part.pop_back();
    } else {
      continue;  // Invalid line format
    }

    NetworkSnapshot snap;
    snap.interface_name = interface_part;

    // Read received stats
    unsigned long long temp_val;
    ss >> snap.rx_bytes >> snap.rx_packets >> temp_val >> temp_val >>
        temp_val >> temp_val >> temp_val >> temp_val;  // Skip errs, drop, etc.

    // Read transmitted stats
    ss >> snap.tx_bytes >> snap.tx_packets;  // Only need these two

    snapshots[snap.interface_name] = snap;
  }
  return snapshots;
}
std::vector<NetworkInterfaceStats> calculate_network_rates(
    const std::map<std::string, NetworkSnapshot>& prev_snapshot,
    const std::map<std::string, NetworkSnapshot>& current_snapshot,
    double time_delta_seconds) {
  std::vector<NetworkInterfaceStats> rates;

  if (time_delta_seconds <= 0.0) {
    return rates;  // Avoid division by zero
  }

  for (const auto& [name, current] : current_snapshot) {
    auto prev_it = prev_snapshot.find(name);
    if (prev_it != prev_snapshot.end()) {
      const auto& prev = prev_it->second;

      NetworkInterfaceStats stats;
      stats.interface_name = name;

      // Check for counter reset (less likely but possible)
      if (current.rx_bytes >= prev.rx_bytes) {
        stats.rx_bytes_per_sec =
            (current.rx_bytes - prev.rx_bytes) / time_delta_seconds;
      }
      if (current.tx_bytes >= prev.tx_bytes) {
        stats.tx_bytes_per_sec =
            (current.tx_bytes - prev.tx_bytes) / time_delta_seconds;
      }

      rates.push_back(stats);
    }
  }
  return rates;
}
void get_network_stats(DataStreamProvider& provider, SystemMetrics& metrics) {
  // Snapshot 1
  auto t1_timestamp = std::chrono::steady_clock::now();
  auto t1_snapshot = read_network_snapshot(provider.get_net_dev_stream());
  // Sleep between reads
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  // Snapshot 2
  auto t2_timestamp = std::chrono::steady_clock::now();
  auto t2_snapshot = read_network_snapshot(provider.get_net_dev_stream());

  // Calculate time delta
  std::chrono::duration<double> time_elapsed = t2_timestamp - t1_timestamp;

  auto calculated_rates =
      calculate_network_rates(t1_snapshot,  // Use snapshot read before sleep
                              t2_snapshot,  // Use snapshot read after sleep
                              time_elapsed.count());

  // Assign rates
  metrics.network_interfaces = calculated_rates;

  // No state needs to be updated for the next call
}
