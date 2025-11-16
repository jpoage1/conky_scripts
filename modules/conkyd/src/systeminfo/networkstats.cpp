// networkstats.cpp
#include "networkstats.hpp"

#include "data.h"
#include "data_local.h"
#include "data_ssh.h"
#include "metrics.hpp"

std::istream& LocalDataStreams::get_net_dev_stream() {
  return create_stream_from_file(net_dev, "/proc/net/dev");
}

std::istream& ProcDataStreams::get_net_dev_stream() {
  return create_stream_from_command(net_dev, "cat /proc/net/dev");
}

NetworkPollingTask::NetworkPollingTask(DataStreamProvider& _provider,
                                       SystemMetrics& _metrics)
    : IPollingTask(_provider, _metrics) {
  name = "Network polling";
}

void NetworkPollingTask::take_snapshot_1() {
  t1_snapshot = read_data(provider.get_net_dev_stream());
}

void NetworkPollingTask::take_snapshot_2() {
  t2_snapshot = read_data(provider.get_net_dev_stream());
}

NetworkSnapshotMap NetworkPollingTask::read_data(std::istream& net_dev_stream) {
  NetworkSnapshotMap snapshots;
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

void NetworkPollingTask::calculate(double time_delta_seconds) {
  std::vector<NetworkInterfaceStats> rates;

  if (time_delta_seconds <= 0.0) {
    metrics.network_interfaces = rates;  // Avoid division by zero
    return;
  }

  for (const auto& [name, current] : t2_snapshot) {
    auto prev_it = t1_snapshot.find(name);
    if (prev_it != t1_snapshot.end()) {
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
  metrics.network_interfaces = rates;
}
