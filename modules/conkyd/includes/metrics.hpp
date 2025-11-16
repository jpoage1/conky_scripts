// metrics.hpp
#pragma once

#include "data.h"
#include "pcn.hpp"

struct CombinedMetrics;

// int get_local_metrics(DataStreamProvider& provider,
//                       const std::string& config_file, CombinedMetrics&
//                       metrics);
// int get_metrics_from_provider(DataStreamProvider& provider,
//                               const std::string& config_file,
//                               CombinedMetrics& metrics);

// int get_server_metrics(DataStreamProvider& provider,
//                        const std::string& config_file,
//                        CombinedMetrics& metrics);

// int get_server_metrics(DataStreamProvider& provider,
//                        const std::string& config_file, CombinedMetrics&
//                        metrics, const std::string& host, const std::string&
//                        user);

int get_local_metrics(DataStreamProviderPtr&, const std::string& config_file,
                      CombinedMetrics& metrics);
int get_metrics_from_provider(DataStreamProviderPtr&,
                              const std::string& config_file,
                              CombinedMetrics& metrics);
int get_server_metrics(DataStreamProviderPtr&, const std::string& config_file,
                       CombinedMetrics& metrics);

int get_server_metrics(DataStreamProviderPtr&, const std::string& config_file,
                       CombinedMetrics& metrics, const std::string& host,
                       const std::string& user);
