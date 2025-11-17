
#include "diskstat.hpp"

#include "colors.hpp"
#include "conky_format.hpp"
#include "data.hpp"
#include "data_local.hpp"
#include "data_ssh.hpp"
#include "filesystems.hpp"
#include "json_definitions.hpp"
#include "polling.hpp"
#include "ssh.hpp"

std::istream& LocalDataStreams::get_diskstats_stream() {
  return create_stream_from_file(diskstats, "/proc/diskstats");
}

std::istream& ProcDataStreams::get_diskstats_stream() {
  return create_stream_from_command(diskstats, "cat /proc/diskstats");
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
void DiskPollingTask::commit() {
  metrics.disks.clear();
  for (const auto& device_path : metrics.device_paths) {
    DeviceInfo info;
    // DiskIO io = get_disk_io_per_sec(device_path);
    // std::cerr << "Reading filesystem device `" << device_path << "`"
    //           << std::endl;
    info.device_path = device_path;
    info.mount_point =
        get_mount_point(provider.get_mounts_stream(), device_path);
    DiskUsage usage;
    if (info.mount_point != "") {
      usage = provider.get_disk_usage(info.mount_point);
      info.used_bytes = usage.used_bytes;
      info.size_bytes = usage.size_bytes;
    }
    std::string device_name;
    try {
      // Resolve the symlink: /dev/bork/nixos -> /dev/dm-0
      std::filesystem::path real_path = std::filesystem::canonical(device_path);
      // Get the filename of the real path: "dm-0"
      device_name = real_path.filename().string();
      //   std::cerr << "Resolved `" << device_path << "` to `" << device_name
      //   << "`"
      //             << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Warning: Could not resolve device path: " << device_path
                << ": " << e.what() << std::endl;
      // Fallback to old logic? Or just leave empty?
      // Leaving it empty will cause it to correctly show "No stats".
    }
    // json output_dump = metrics.disk_io_rates;
    // std::cerr << output_dump;

    // Find the pre-calculated stats from the metrics map
    auto it = metrics.disk_io_rates.find(device_name);
    if (it != metrics.disk_io_rates.end()) {
      //   std::cerr << "Setting values.." << std::endl;
      const auto& disk_stats = it->second;
      // Format the uint64_t values into strings as required by DeviceInfo
      info.read_bytes_per_sec = format_size(disk_stats.read_bytes_per_sec).text;
      info.write_bytes_per_sec =
          format_size(disk_stats.write_bytes_per_sec).text;
    } else {
      //   std::cerr << "No stats.." << std::endl;
      // Default to "0" if no stats were found for this device
      info.read_bytes_per_sec = format_size(0).text;
      info.write_bytes_per_sec = format_size(0).text;
    }
    metrics.disks.push_back(info);
  }
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
      metrics.disk_io_rates[dev_name] = {
          .device_name = dev_name,  // Still good to store the name here
          .read_bytes_per_sec =
              static_cast<uint64_t>(read_delta / time_delta_seconds),
          .write_bytes_per_sec =
              static_cast<uint64_t>(write_delta / time_delta_seconds)};
    }
  }
}
namespace {
const DiskStatConfig config;
// my_settings.insert(DiskStatSettings::Loopback);
// my_settings.insert(DiskStatSettings::MapperDevices);
// mysettings.insert(DiskStatSettings::Partitions);
};  // namespace
DiskIoSnapshotMap DiskPollingTask::read_data(std::istream& diskstats_stream) {
  DiskIoSnapshotMap snapshots;

  diskstats_stream.clear();
  diskstats_stream.seekg(0, std::ios::beg);

  //   std::cerr << "[DEBUG] read_data: Reading /proc/diskstats..." <<
  //   std::endl;

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

    // // --- DEBUG INFO ---
    // std::cerr << "[DEBUG] Line " << line_count << ": "
    //           << "dev=" << dev_name << ", "
    //           << "major=" << major << ", "
    //           << "minor=" << minor;

    // // Filter 1: Check Loopback devices
    // if (major == 7 && config.count(DiskStatSettings::Loopback) == 0) {
    //   continue;  // Skip if major is 7 and Loopback is NOT in the set
    // }

    // // Filter 2: Check Device-Mapper
    // if (dev_name.rfind("dm-", 0) == 0 &&
    //     config.count(DiskStatSettings::MapperDevices) == 0) {
    //   continue;  // Skip if "dm-" and MapperDevices is NOT in the set
    // }

    // // Filter 3: Check Partitions
    // if (!dev_name.empty() && std::isdigit(dev_name.back()) &&
    //     config.count(DiskStatSettings::Partitions) == 0) {
    //   continue;  // Skip if it's a partition and Partitions is NOT in the set
    // }

    //   std::cerr << " -> ADDING" << std::endl;
    snapshots[dev_name] = {.bytes_read = sectors_read * 512,
                           .bytes_written = sectors_written * 512};
  }

  //   std::cerr << "[DEBUG] read_data: Finished. Collected " <<
  //   snapshots.size()
  //             << " devices." << std::endl;

  return snapshots;
}

std::string get_mount_point(std::istream& mounts_stream,
                            const std::string& device_path) {
  std::string device, mount_point, rest;

  // 1. Calculate the canonical path for the device we are looking for once.
  std::filesystem::path target_path_canonical;
  try {
    target_path_canonical = std::filesystem::canonical(device_path);
  } catch (const std::filesystem::filesystem_error& e) {
    // If the target device_path doesn't exist, we can't find it.
    return "";
  }

  while (mounts_stream >> device >> mount_point) {
    std::getline(mounts_stream, rest);

    // 2. Use the memory-safe std::filesystem::canonical for comparison.
    try {
      std::filesystem::path current_dev_canonical =
          std::filesystem::canonical(device);

      // Compare the canonical paths.
      if (current_dev_canonical == target_path_canonical) {
        return mount_point;
      }
    } catch (const std::filesystem::filesystem_error& e) {
      // Ignore errors for devices like "proc" or "sysfs" that may not resolve.
      continue;
    }
  }
  return "";
}

std::istream& LocalDataStreams::get_mounts_stream() {
  return create_stream_from_file(mounts, "/proc/mounts");
}

std::istream& ProcDataStreams::get_mounts_stream() {
  return create_stream_from_command(mounts, "cat /proc/mounts");
}

DiskUsage LocalDataStreams::get_disk_usage(const std::string& mount_point) {
  struct statvfs stat;
  DiskUsage usage;
  if (!mount_point.empty() && statvfs(mount_point.c_str(), &stat) == 0) {
    usage.used_bytes = (stat.f_blocks - stat.f_bfree) * stat.f_frsize;
    usage.size_bytes = stat.f_blocks * stat.f_frsize;
  }
  return usage;
}

DiskUsage ProcDataStreams::get_disk_usage(const std::string& mount_point) {
  DiskUsage usage;
  // Execute df command and get the output as a single string.
  std::string df_output = execute_ssh_command("df -B1 " + mount_point);

  // Check if the command was successful. df returns an error if the mount point
  // is not found.
  if (df_output.empty()) {
    std::cerr << "Error: Could not get df output for mount point "
              << mount_point << std::endl;
    return usage;
  }

  std::stringstream df_stream(df_output);
  std::string line;
  std::getline(df_stream, line);  // Read and discard the header line.

  std::getline(df_stream, line);  // Read the data line.

  std::stringstream data_stream(line);
  std::string filesystem, blocks, used, available, capacity, mounted_on;

  // Parse the data line.
  if (data_stream >> filesystem >> blocks >> used >> available >> capacity >>
      mounted_on) {
    // df might return multiple lines if the mount point is a symbolic link.
    // We'll trust the first data line.
    try {
      usage.used_bytes = std::stoull(used);
      usage.size_bytes = std::stoull(blocks);
    } catch (const std::exception& e) {
      std::cerr << "Error parsing numbers from df output: " << e.what()
                << std::endl;
      return usage;
    }
  }

  std::cerr << "Error: Could not parse df output for mount point "
            << mount_point << std::endl;
  return usage;
}
