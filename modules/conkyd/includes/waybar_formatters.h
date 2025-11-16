// waybar_formatters.hpp
#pragma once

#include "pcn.hpp"
#include "runner.hpp"
#include "waybar_types.h"

FormattedSize format_size_rate(double bytes_per_sec);
void generate_waybar_output(const std::vector<MetricsContext>& all_results);

std::string show_top_mem_procs(
    const MetricsContext& result,
    const std::vector<ProcessInfo>& top_processes_mem);

std::string show_top_cpu_procs(const MetricsContext& result,
                               const std::vector<ProcessInfo>& top_procs_cpu);

std::string show_network_interfaces(
    const MetricsContext& result,
    const std::vector<NetworkInterfaceStats>& network_interfaces,
    const std::set<std::string>& specific_interfaces);

std::string show_devices(const MetricsContext& result,
                         const std::vector<DeviceInfo>& devices);

std::string show_system_metrics(const MetricsContext& result,
                                const SystemMetrics& system_metrics,
                                int& total_mem_percent, int& valid_mem_sources);
