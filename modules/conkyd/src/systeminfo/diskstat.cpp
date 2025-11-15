
#include "diskstat.hpp"

#include <iostream>
#include <istream>
#include <map>
#include <sstream>
#include <string>

#include "data_local.h"
#include "data_ssh.h"

std::istream& LocalDataStreams::get_diskstats_stream() {
  reset_stream(diskstats, "/proc/diskstats");
  return diskstats;
}

std::istream& ProcDataStreams::get_diskstats_stream() {
  std::string diskstats_data = execute_ssh_command("cat /proc/diskstats");
  diskstats.str(diskstats_data);
  rewind(diskstats, "diskstats");
  return diskstats;
}

DiskPollingTask::DiskPollingTask(DataStreamProvider& _provider,
                                 SystemMetrics& _metrics)
    : IPollingTask(_provider, _metrics) {}

// PollingTaskList read_data(DataStreamProvider&, SystemMetrics&);
void DiskPollingTask::take_snapshot_1() {
  t1_snapshots = read_data(provider.get_diskstats_stream());
}
void DiskPollingTask::take_snapshot_2() {
  t2_snapshots = read_data(provider.get_diskstats_stream());
}

void DiskPollingTask::calculate(double time_delta_seconds) {
  // Clear old rates and fill with new ones
  metrics.disk_io_rates.clear();

  if (time_delta_seconds <= 0) return;

  for (auto const& [dev_name, t2_snap] : t2_snapshots) {
    auto t1_it = t1_snapshots.find(dev_name);
    if (t1_it != t1_snapshots.end()) {
      const auto& t1_snap = t1_it->second;

      // Calculate deltas
      uint64_t read_delta = (t2_snap.bytes_read >= t1_snap.bytes_read)
                                ? (t2_snap.bytes_read - t1_snap.bytes_read)
                                : 0;

      uint64_t write_delta =
          (t2_snap.bytes_written >= t1_snap.bytes_written)
              ? (t2_snap.bytes_written - t1_snap.bytes_written)
              : 0;

      // Add the calculated rate to our metrics
      metrics.disk_io_rates.push_back(
          {.device_name = dev_name,
           .read_bytes_per_sec =
               static_cast<uint64_t>(read_delta / time_delta_seconds),
           .write_bytes_per_sec =
               static_cast<uint64_t>(write_delta / time_delta_seconds)});
    }
  }
}
DiskIoSnapshotMap DiskPollingTask::read_data(std::istream& diskstats_stream) {
  DiskIoSnapshotMap snapshots;

  diskstats_stream.clear();
  diskstats_stream.seekg(0, std::ios::beg);

  std::cerr << "[DEBUG] read_data: Reading /proc/diskstats..." << std::endl;

  std::string line;
  int line_count = 0;
  while (std::getline(diskstats_stream, line)) {
    line_count++;
    std::istringstream iss(line);
    int major, minor;
    std::string dev_name;
    uint64_t reads_completed, reads_merged, sectors_read, time_reading;
    uint64_t writes_completed, writes_merged, sectors_written, time_writing;

    iss >> major >> minor >> dev_name >> reads_completed >> reads_merged >>
        sectors_read >> time_reading >> writes_completed >> writes_merged >>
        sectors_written >> time_writing;

    // --- DEBUG INFO ---
    std::cerr << "[DEBUG] Line " << line_count << ": "
              << "dev=" << dev_name << ", "
              << "major=" << major << ", "
              << "minor=" << minor;

    // The filter you are testing
    if (major > 0) {
      std::cerr << " -> ADDING" << std::endl;
      snapshots[dev_name] = {.bytes_read = sectors_read * 512,
                             .bytes_written = sectors_written * 512};
    } else {
      std::cerr << " -> SKIPPING (major == 0)" << std::endl;
    }
    // --- END DEBUG ---
  }

  std::cerr << "[DEBUG] read_data: Finished. Collected " << snapshots.size()
            << " devices." << std::endl;

  return snapshots;
}
