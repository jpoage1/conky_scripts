
#include "diskstat.hpp"

#include "data.hpp"
#include "data_local.hpp"
#include "data_ssh.hpp"
#include "filesystems.hpp"
#include "json_definitions.hpp"
#include "polling.hpp"
#include "runner.hpp"
#include "ssh.hpp"

std::istream& LocalDataStreams::get_diskstats_stream() {
  return create_stream_from_file(diskstats, "/proc/diskstats");
}

std::istream& ProcDataStreams::get_diskstats_stream() {
  return create_stream_from_command(diskstats, "cat /proc/diskstats");
}

DevicePaths DiskPollingTask::load_device_paths(const std::string& config_file) {
  DevicePaths device_paths;

  namespace fs = std::filesystem;

  // Check if the file exists, is a regular file, and is readable
  if (!fs::exists(config_file) || !fs::is_regular_file(config_file)) {
    std::cerr << "Unable to load device path file: " + config_file << std::endl;
    return device_paths;  // Return false on failure
  }

  std::ifstream file(config_file);
  if (!file.is_open()) {
    std::cerr << "Unable to open device path file: " + config_file << std::endl;
    return device_paths;
  }

  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty()) {
      device_paths.push_back(line);
    }
  }
  return device_paths;  // Return true on success
}
void DiskPollingTask::configure() {}
DiskPollingTask::DiskPollingTask(DataStreamProvider& provider,
                                 SystemMetrics& metrics,
                                 MetricsContext& context)
    : IPollingTask(provider, metrics, context) {
  // 1. Copy the Config
  this->config = context.disk_stat_config;

  // 2. Load the Allowlist
  for (const auto& dev : context.io_devices) {
    // Strip "/dev/" prefix if the user included it in Lua
    std::string clean_name = dev;
    size_t pos = clean_name.rfind("/dev/");
    if (pos != std::string::npos) {
      clean_name = clean_name.substr(pos + 5);
    }
    allowed_io_devices.insert(clean_name);
  }
  DevicePaths device_paths;
  device_paths.insert(device_paths.end(), context.filesystems.begin(),
                      context.filesystems.end());

  if (!context.device_file.empty()) {
    std::vector<std::string> loaded = load_device_paths(context.device_file);
    device_paths.insert(device_paths.end(), loaded.begin(), loaded.end());
  }

  // 2. Iterate the paths we JUST loaded
  for (const std::string& logical_path : device_paths) {
    try {
      std::filesystem::path real_path =
          std::filesystem::canonical(logical_path);
      std::string kernel_name = real_path.filename().string();

      // Store the kernel name for read_data() filtering
      target_kernel_names.insert(kernel_name);

      // Build the persistent "skeleton" in SystemMetrics
      DeviceInfo info;
      info.device_path = logical_path;
      info.mount_point =
          get_mount_point(provider.get_mounts_stream(), logical_path);
      //   std::cerr << "Found mount point: " << info.mount_point << std::endl;
      info.usage = provider.get_disk_usage(info.mount_point);

      metrics.disks.push_back(std::move(info));

      // Store a pointer to the object we just created
      DeviceInfo* device_ptr = &metrics.disks.back();
      kernel_to_device_map[kernel_name] = device_ptr;

    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Warning (DiskPollingTask): Could not resolve device path: "
                << logical_path << ": " << e.what() << std::endl;
    }
  }
}
// PollingTaskList read_data(DataStreamProvider&, SystemMetrics&);
void DiskPollingTask::take_snapshot_1() {
  t1_snapshots = read_data(provider.get_diskstats_stream());
}
void DiskPollingTask::take_snapshot_2() {
  t2_snapshots = read_data(provider.get_diskstats_stream());
}

void DiskPollingTask::commit() { t1_snapshots = t2_snapshots; }

void DiskPollingTask::calculate(double time_delta_seconds) {
  //   std::cerr << "CALCULATE START: &metrics = " << &metrics
  //             << ", disk_io size = " << metrics.disk_io.size()
  //             << ", &disk_io = " << &metrics.disk_io << std::endl;

  metrics.disk_io.clear();

  if (time_delta_seconds <= 0) return;

  // 1. Reset I/O stats for config-file devices
  for (const auto& [kernel_name, info_ptr] : kernel_to_device_map) {
    info_ptr->io.read_bytes_per_sec = 0;
    info_ptr->io.write_bytes_per_sec = 0;
  }

  // 3. Calculate stats for all devices
  for (auto const& [dev_name, t2_snap] : t2_snapshots) {
    auto t1_it = t1_snapshots.find(dev_name);
    if (t1_it == t1_snapshots.end()) {
      continue;  // No T1 data, skip
    }

    const auto& t1_snap = t1_it->second;
    uint64_t read_delta = (t2_snap.bytes_read >= t1_snap.bytes_read)
                              ? (t2_snap.bytes_read - t1_snap.bytes_read)
                              : 0;
    uint64_t write_delta = (t2_snap.bytes_written >= t1_snap.bytes_written)
                               ? (t2_snap.bytes_written - t1_snap.bytes_written)
                               : 0;
    uint64_t read_bps = static_cast<uint64_t>(read_delta / time_delta_seconds);
    uint64_t write_bps =
        static_cast<uint64_t>(write_delta / time_delta_seconds);

    // Check if this device is one of the ones from the config file
    auto info_it = kernel_to_device_map.find(dev_name);
    if (info_it != kernel_to_device_map.end()) {
      // It is. Update the DeviceInfo struct directly.
      DeviceInfo* info_ptr = info_it->second;
      info_ptr->io.read_bytes_per_sec = read_bps;
      info_ptr->io.write_bytes_per_sec = write_bps;
    } else {
      // It's not from the config. Add it to the generic HdIoStats map.
      HdIoStats& io = metrics.disk_io[dev_name];
      io.device_name = dev_name;
      io.read_bytes_per_sec = read_bps;
      io.write_bytes_per_sec = write_bps;
      //   std::cerr << "Found disk `" << io.device_name << "'" << std::endl;
      //   std::cerr << "CALCULATE: Added to disk_io map: " << dev_name
      //             << " (read: " << read_bps << ", write: " << write_bps
      //             << "), map size: " << metrics.disk_io.size() << std::endl;
    }
  }
  //   std::cerr << "CALCULATE END: disk_io map has " << metrics.disk_io.size()
  //             << " entries" << std::endl;
  //   for (const auto& [name, stats] : metrics.disk_io) {
  //     std::cerr << "  - " << name << ": '" << stats.device_name
  //               << "' read=" << stats.read_bytes_per_sec
  //               << " write=" << stats.write_bytes_per_sec << std::endl;
  //   }
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

    bool keep = false;

    // RULE 1: Always keep devices that map to our Filesystems
    if (target_kernel_names.count(dev_name) > 0) {
      keep = true;
    }
    // RULE 2: Always keep devices explicitly requested in Lua 'io_devices'
    else if (allowed_io_devices.count(dev_name) > 0) {
      keep = true;
    }
    // RULE 3: If Strict Mode is active (io_devices is not empty), SKIP
    // everything else
    else if (!allowed_io_devices.empty()) {
      keep = false;
    }
    // RULE 4: Auto-Discovery Filters (Only runs if strict mode is OFF)
    else {
      keep = true;  // Assume true, then try to disqualify

      // Filter: Loopback
      if (major == 7 && config.count(DiskStatSettings::Loopback) == 0)
        keep = false;

      // Filter: Device-Mapper
      if (dev_name.rfind("dm-", 0) == 0 &&
          config.count(DiskStatSettings::MapperDevices) == 0)
        keep = false;

      // Filter: Partitions (ends in digit)
      if (!dev_name.empty() && std::isdigit(dev_name.back()) &&
          config.count(DiskStatSettings::Partitions) == 0)
        keep = false;
    }

    if (keep) {
      snapshots[dev_name] = {.bytes_read = sectors_read * 512,
                             .bytes_written = sectors_written * 512};
    }
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
