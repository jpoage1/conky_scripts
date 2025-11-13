// metrics.hpp
#pragma once

#include <string>

#include "data.h"
#include "types.h"

struct CombinedMetrics {
  std::vector<std::unique_ptr<IPollingTask>> polled;
  SystemMetrics system;
  std::vector<DeviceInfo> disks;
};


struct PollingMetrics {
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
    std::vector<CpuSnapshot> cpu_snapshots;
    std::map<std::string, NetworkSnapshot> network_snapshots;

    PollingMetrics(DataStreamProvider &provider) {
        timestamp = std::chrono::steady_clock::now();
        cpu_snapshots = read_cpu_snapshots(provider.get_stat_stream());
        network_snapshots = read_network_snapshot(provider.get_net_dev_stream());
    }
};

int get_metrics(const std::string&, const bool);

int get_metrics(const std::string& config_file, const bool use_ssh,
                CombinedMetrics& metrics);

int get_local_metrics(const std::string& config_file, CombinedMetrics& metrics);
int get_metrics_from_provider(DataStreamProvider& provider,
                              const std::string& config_file,
                              CombinedMetrics& metrics);

int get_server_metrics(const std::string& config_file,
                       CombinedMetrics& metrics);

int get_server_metrics(const std::string& config_file, CombinedMetrics& metrics,
                       const std::string& host, const std::string& user);

void waybar_local_metrics(const std::string& config_file);
void waybar_server_metrics(const std::string& config_file);
void waybar_server_metrics(const std::string& config_file,
                           const std::string& host, const std::string& user);
