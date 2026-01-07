// networkstats.cpp
#include "networkstats.hpp"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>

#include "data_local.hpp"
#include "data_ssh.hpp"
#include "lua_generator.hpp"
#include "metrics.hpp"
#include "pcn.hpp"
#include "polling.hpp"
#include "stream_provider.hpp"

namespace telemetry {

std::istream &LocalDataStreams::get_net_dev_stream() {
  return create_stream_from_file(net_dev, "/proc/net/dev");
}

std::istream &ProcDataStreams::get_net_dev_stream() {
  return create_stream_from_command(net_dev, "cat /proc/net/dev");
}

NetworkPollingTask::NetworkPollingTask(DataStreamProvider &provider,
                                       SystemMetrics &metrics,
                                       MetricsContext &context)
    : IPollingTask(provider, metrics, context) {
  name = "Network polling";
}

void NetworkPollingTask::take_initial_snapshot() {
  set_timestamp();
  prev_snapshot = read_data(provider.get_net_dev_stream());
}

void NetworkPollingTask::take_new_snapshot() {
  set_delta_time();
  current_snapshot = read_data(provider.get_net_dev_stream());
}
void NetworkPollingTask::commit() {
  prev_snapshot = current_snapshot; // Note: singular 'snapshot' based on your
                                    // previous code
}

NetworkSnapshotMap NetworkPollingTask::read_data(std::istream &net_dev_stream) {
  NetworkSnapshotMap snapshots;
  std::string line;

  // Skip the first two header lines
  std::getline(net_dev_stream, line);
  std::getline(net_dev_stream, line);

  while (std::getline(net_dev_stream, line)) {
    std::stringstream ss(line);
    std::string interface_part;
    ss >> interface_part; // e.g., "eth0:" or " face |bytes    packets errs
                          // drop fifo frame compressed multicast|bytes ..."

    // Remove trailing colon if present
    if (!interface_part.empty() && interface_part.back() == ':') {
      interface_part.pop_back();
    } else {
      continue; // Invalid line format
    }

    NetworkSnapshot snap;
    snap.interface_name = interface_part;

    // Read received stats
    unsigned long long temp_val;
    ss >> snap.rx_bytes >> snap.rx_packets >> temp_val >> temp_val >>
        temp_val >> temp_val >> temp_val >> temp_val; // Skip errs, drop, etc.

    // Read transmitted stats
    ss >> snap.tx_bytes >> snap.tx_packets; // Only need these two

    snapshots[snap.interface_name] = snap;
  }
  return snapshots;
}

void NetworkPollingTask::calculate() {
  std::vector<NetworkInterfaceStats> rates;

  if (time_delta_seconds <= 0.0) {
    metrics.network_interfaces = rates; // Avoid division by zero
    return;
  }

  for (const auto &[name, current] : current_snapshot) {
    auto prev_it = prev_snapshot.find(name);
    if (prev_it != prev_snapshot.end()) {
      const auto &prev = prev_it->second;

      NetworkInterfaceStats stats;
      stats.interface_name = name;
      stats.ip_address = get_ip_address(name);

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
std::string get_ip_address(const std::string &interface_name) {
  struct ifaddrs *ifaddr, *ifa;
  std::string ip = "";

  if (getifaddrs(&ifaddr) == -1)
    return "";

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;
    if (ifa->ifa_addr->sa_family == AF_INET) { // IPv4
      if (interface_name == ifa->ifa_name) {
        char host[NI_MAXHOST];
        int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host,
                            NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if (s == 0)
          ip = host;
        break;
      }
    }
  }
  freeifaddrs(ifaddr);
  return ip;
}

Network::Network() {
  const std::string net_dir = "/sys/class/net";
  if (!fs::exists(net_dir))
    return;

  for (const auto &entry : fs::directory_iterator(net_dir)) {
    std::string iface = entry.path().filename().string();

    // 1. Skip loopback
    if (iface == "lo")
      continue;

    // 2. Filter for physical devices
    // On Linux, virtual interfaces (lo, veth, br0) lack a 'device'
    // symlink
    if (fs::exists(entry.path() / "device")) {
      interfaces.push_back(iface);
    }
  }

  // Optional: Sort alphabetically for consistent Lua output
  std::sort(interfaces.begin(), interfaces.end());
}

bool Network::has_interfaces() const { return !interfaces.empty(); }

std::string LuaNetwork::serialize(unsigned indentation_level) const {
  LuaConfigGenerator gen("network", indentation_level);

  // Serialize interfaces as a Lua table array
  gen.lua_vector("interfaces", interfaces);
  gen.lua_string("ping_target", ping_target);
  gen.lua_bool("enable_ping", enable_ping);

  return gen.str();
} // End Network::serialize()

void LuaNetwork::deserialize(sol::table net) {
  // Extract Interface List
  if (net["interfaces"].valid()) {
    // Auto-convert Lua array to C++ vector
    interfaces = net["interfaces"].get<std::vector<std::string>>();
  }
}

}; // namespace telemetry
