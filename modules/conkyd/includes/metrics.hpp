// metrics.hpp
#pragma once

#include <string>

#include "data.h"
#include "types.h"

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
