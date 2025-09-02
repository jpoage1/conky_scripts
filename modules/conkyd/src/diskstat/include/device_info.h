#pragma once

#include <string>
#include <vector>

#include "data.h"
#include "types.h"

std::vector<DeviceInfo> collect_device_info(
    const std::vector<std::string> &device_paths);
std::vector<DeviceInfo> collect_device_info(
    DataStreamProvider &provider, const std::vector<std::string> &device_paths);
uint64_t get_used_space_bytes(const std::string &mount_point);
uint64_t get_disk_size_bytes(const std::string &mount_point);
