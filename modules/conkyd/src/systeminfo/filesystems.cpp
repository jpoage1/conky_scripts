#include "filesystems.hpp"

#include "colors.h"
#include "conky_format.h"
#include "data.h"
#include "data_local.h"
#include "data_ssh.h"
#include "filesystems.hpp"
#include "ssh.h"

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

// uint64_t LocalDataStreams::get_used_space_bytes(
//     const std::string& mount_point) {
//   struct statvfs stat;
//   if (mount_point.empty() || statvfs(mount_point.c_str(), &stat) != 0) return
//   0; return (stat.f_blocks - stat.f_bfree) * stat.f_frsize;
// }

// uint64_t LocalDataStreams::get_disk_size_bytes(const std::string&
// mount_point) {
//   struct statvfs stat;
//   if (mount_point.empty() || statvfs(mount_point.c_str(), &stat) != 0) return
//   0; return stat.f_blocks * stat.f_frsize;
// }

std::vector<DeviceInfo> collect_device_info(
    DataStreamProvider& provider,
    const std::vector<std::string>& device_paths) {
  std::vector<DeviceInfo> data;
  for (const auto& device_path : device_paths) {
    DeviceInfo info;
    // DiskIO io = get_disk_io_per_sec(device_path);
    info.device_path = device_path;
    info.mount_point =
        get_mount_point(provider.get_mounts_stream(), device_path);
    DiskUsage usage;
    if (info.mount_point != "") {
      provider.get_disk_usage(info.mount_point);
      info.used_bytes = usage.used_bytes;
      info.size_bytes = usage.size_bytes;
    }
    // info.read_bytes_per_sec = io.read_per_sec;
    // info.write_bytes_per_sec = io.write_per_sec;
    data.push_back(info);
  }
  return data;
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

static std::unordered_map<std::string, DeviceStats> stats_map;

static uint64_t current_time_in_ms() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
      .count();
}
std::string get_read_bytes_per_sec(std::istream& diskstats_stream,
                                   const std::string& device_path) {
  if (device_path.empty()) {
    return "";
  }

  std::string device_name =
      device_path.substr(device_path.find_last_of('/') + 1);
  while (!device_name.empty() && isdigit(device_name.back())) {
    device_name.pop_back();
  }

  uint64_t current_bytes_read = 0;

  std::string line;

  diskstats_stream.seekg(0);
  diskstats_stream.clear();

  while (std::getline(diskstats_stream, line)) {
    std::istringstream iss(line);
    std::string dev;
    uint64_t sectors = 0;
    int field = 0;
    while (iss >> dev) {
      if (++field == 3)
        dev = dev;
      else if (field == 6)
        sectors = std::stoull(dev);
    }
    if (dev == device_name) {
      current_bytes_read = sectors * 512;
      break;
    }
  }

  uint64_t now = current_time_in_ms();
  auto& stat = stats_map[device_name];
  uint64_t delta_bytes = 0;
  uint64_t delta_ms = 1;

  if (stat.prev_timestamp_ms && now > stat.prev_timestamp_ms) {
    delta_bytes = current_bytes_read - stat.prev_bytes_read;
    delta_ms = now - stat.prev_timestamp_ms;
  }

  stat.prev_bytes_read = current_bytes_read;
  stat.prev_timestamp_ms = now;
  return format_size((delta_bytes * 1000) / delta_ms).text;
}

std::string get_write_bytes_per_sec(std::istream& diskstats_stream,
                                    const std::string& device_path) {
  if (device_path.empty()) {
    return "";
  }

  std::string device_name =
      device_path.substr(device_path.find_last_of('/') + 1);
  while (!device_name.empty() && isdigit(device_name.back())) {
    device_name.pop_back();
  }

  uint64_t current_bytes_written = 0;
  std::string line;

  diskstats_stream.seekg(0);
  diskstats_stream.clear();

  while (std::getline(diskstats_stream, line)) {
    std::istringstream iss(line);
    std::string dev;
    uint64_t sectors = 0;
    int field = 0;
    while (iss >> dev) {
      if (++field == 3)
        dev = dev;
      else if (field == 10)
        sectors = std::stoull(dev);
    }
    if (dev == device_name) {
      current_bytes_written = sectors * 512;
      break;
    }
  }

  uint64_t now = current_time_in_ms();
  auto& stat = stats_map[device_name];
  uint64_t delta_bytes = 0;
  uint64_t delta_ms = 1;

  if (stat.prev_timestamp_ms && now > stat.prev_timestamp_ms) {
    delta_bytes = current_bytes_written - stat.prev_bytes_written;
    delta_ms = now - stat.prev_timestamp_ms;
  }

  stat.prev_bytes_written = current_bytes_written;
  stat.prev_timestamp_ms = now;

  return format_size((delta_bytes * 1000) / delta_ms).text;
}

DiskIO get_disk_io_per_sec(const std::string& device_path) {
  std::ifstream diskstats("/proc/diskstats");
  return get_disk_io_per_sec(diskstats, device_path);
}

DiskIO get_disk_io_per_sec(std::istream& diskstats_stream,
                           const std::string& device_path) {
  if (device_path.empty()) return {};

  std::string device_name =
      device_path.substr(device_path.find_last_of('/') + 1);
  while (!device_name.empty() && isdigit(device_name.back()))
    device_name.pop_back();

  uint64_t current_bytes_read = 0;
  uint64_t current_bytes_written = 0;

  std::string line;

  diskstats_stream.seekg(0);
  diskstats_stream.clear();

  while (std::getline(diskstats_stream, line)) {
    std::istringstream iss(line);
    int major, minor;
    std::string dev;
    uint64_t reads_completed, reads_merged, sectors_read, time_reading;
    uint64_t writes_completed, writes_merged, sectors_written, time_writing;
    iss >> major >> minor >> dev >> reads_completed >> reads_merged >>
        sectors_read >> time_reading >> writes_completed >> writes_merged >>
        sectors_written >> time_writing;
    if (dev == device_name) {
      current_bytes_read = sectors_read * 512;
      current_bytes_written = sectors_written * 512;
      break;
    }
  }

  uint64_t now = current_time_in_ms();
  auto& stat = stats_map[device_name];
  uint64_t delta_read = 0, delta_write = 0;
  uint64_t delta_ms = 1;

  if (stat.prev_timestamp_ms && now > stat.prev_timestamp_ms) {
    delta_read = current_bytes_read - stat.prev_bytes_read;
    delta_write = current_bytes_written - stat.prev_bytes_written;
    delta_ms = now - stat.prev_timestamp_ms;
  }

  stat.prev_bytes_read = current_bytes_read;
  stat.prev_bytes_written = current_bytes_written;
  stat.prev_timestamp_ms = now;

  return {format_size((delta_read * 1000) / delta_ms).text,
          format_size((delta_write * 1000) / delta_ms).text};
}
