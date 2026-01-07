// provider.hpp
#ifndef PROVIDER_HPP
#define PROVIDER_HPP

#include "batteryinfo.hpp"
#include "pcn.hpp"

namespace telemetry {

struct DiskUsage;
struct ProcessRawSnapshot;
struct BatteryStatus;
struct Batteries;

using ProcessSnapshotMap = std::map<long, ProcessRawSnapshot>;

enum DataStreamProviders {
  LocalDataStream,
  ProcDataStream,
};

class DataStreamProvider {
public:
  void rewind(std::istream &stream, std::string streamName);
  void rewind(std::istream &stream);

  virtual ~DataStreamProvider() = default;
  virtual std::vector<BatteryStatus> get_battery_status(const Batteries &) = 0;
  virtual std::istream &get_cpuinfo_stream() = 0;
  virtual std::istream &get_meminfo_stream() = 0;
  virtual std::istream &get_uptime_stream() = 0;
  virtual std::istream &get_stat_stream() = 0;
  virtual std::istream &get_mounts_stream() = 0;
  virtual std::istream &get_diskstats_stream() = 0;
  virtual std::istream &get_loadavg_stream() = 0;
  virtual std::istream &get_net_dev_stream() = 0;
  virtual ProcessSnapshotMap get_process_snapshots(bool) = 0;
  //   virtual std::istream& get_top_mem_processes_stream() = 0;
  //   virtual std::istream& get_top_cpu_processes_stream() = 0;
  virtual DiskUsage get_disk_usage(const std::string &) = 0;
  //   virtual uint64_t get_used_space_bytes(const std::string& mount_point) =
  //   0; virtual uint64_t get_disk_size_bytes(const std::string& mount_point) =
  //   0;
  virtual double get_cpu_temperature() = 0;
  virtual void cleanup() = 0;
};

using DataStreamProviderPtr = std::unique_ptr<DataStreamProvider>;

struct ProviderSettings {
  std::string type; // e.g., "ssh", "local", "rest"
  std::string name; // The user-defined name for this instance
  bool enabled = true;

  // Use a map for dynamic keys like 'host', 'user', 'path'
  std::map<std::string, std::string> parameters;
};

struct LuaProviderSettings : public ProviderSettings {
  std::string serialize(int indentation_level = 0) const;
  void deserialize(const sol::table &provider_table);
};

}; // namespace telemetry
#endif
