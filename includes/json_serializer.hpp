// json_serializer.hpp
#pragma once
#include <functional>
#include <nlohmann/json.hpp>
#include <vector>

#include "metrics.hpp"

class JsonSerializer {
 public:
  using PipelineTask =
      std::function<void(nlohmann::json&, const SystemMetrics&)>;

 private:
  std::vector<PipelineTask> pipeline;

 public:
  // Constructor builds the pipeline ONCE based on settings
  JsonSerializer(const MetricSettings& settings) {
    // 1. Static Metadata (Always included)
    pipeline.emplace_back([](nlohmann::json& j, const SystemMetrics& s) {
      j["sys_name"] = s.sys_name;
      j["node_name"] = s.node_name;
      j["kernel_release"] = s.kernel_release;
      j["machine_type"] = s.machine_type;
    });

    // 2. Conditional Fields
    if (settings.enable_uptime) {
      pipeline.emplace_back([](nlohmann::json& j, const SystemMetrics& s) {
        j["uptime"] = s.uptime;
        j["cpu_frequency_ghz"] = s.cpu_frequency_ghz;
      });
    }

    if (settings.enable_memory) {
      pipeline.emplace_back([](nlohmann::json& j, const SystemMetrics& s) {
        j["meminfo"] = s.meminfo;
        j["swapinfo"] = s.swapinfo;
      });
    }

    if (settings.enable_cpu_temp) {
      pipeline.emplace_back([](nlohmann::json& j, const SystemMetrics& s) {
        j["cpu_temp_c"] = s.cpu_temp_c;
      });
    }

    if (settings.enable_sysinfo) {
      pipeline.emplace_back([](nlohmann::json& j, const SystemMetrics& s) {
        j["cores"] = s.cores;
        j["disks"] = s.disks;
      });
    }

    if (settings.enable_diskstat) {
      pipeline.emplace_back([](nlohmann::json& j, const SystemMetrics& s) {
        j["disk_io"] = nlohmann::json::array();
        for (const auto& pair : s.disk_io) {
          j["disk_io"].push_back(pair.second);
        }
      });
    }

    // 3. Process Lists
    if (settings.enable_avg_processinfo_cpu) {
      pipeline.emplace_back([](nlohmann::json& j, const SystemMetrics& s) {
        j["top_processes_avg_cpu"] = s.top_processes_avg_cpu;
      });
    }
    if (settings.enable_realtime_processinfo_cpu) {
      pipeline.emplace_back([](nlohmann::json& j, const SystemMetrics& s) {
        j["top_processes_real_cpu"] = s.top_processes_real_cpu;
      });
    }
    if (settings.enable_avg_processinfo_mem) {
      pipeline.emplace_back([](nlohmann::json& j, const SystemMetrics& s) {
        j["top_processes_avg_mem"] = s.top_processes_avg_mem;
      });
    }
    if (settings.enable_realtime_processinfo_mem) {
      pipeline.emplace_back([](nlohmann::json& j, const SystemMetrics& s) {
        j["top_processes_real_mem"] = s.top_processes_real_mem;
      });
    }
  }

  // The runtime function - No "if" checks here
  nlohmann::json serialize(const SystemMetrics& metrics) const {
    nlohmann::json j = nlohmann::json::object();
    for (const auto& task : pipeline) {
      task(j, metrics);
    }
    return j;
  }
};
