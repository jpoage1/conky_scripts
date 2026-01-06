#ifndef SYSTEM_METRICS_PROXY_HPP
#define SYSTEM_METRICS_PROXY_HPP
#include <nlohmann/json.hpp>

class SystemMetricsProxy{
    public:
    SystemMetricsProxy() = default;
    virtual ~SystemMetricsProxy() = default;
    // virtual void updateData(nlohmann::json_abi_v3_12_0::json&) = 0;
};

#endif
