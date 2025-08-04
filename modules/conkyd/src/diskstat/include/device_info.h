#pragma once

#include "types.h"
#include <vector>
#include <string>

std::vector<DeviceInfo> collect_device_info(const std::vector<std::string> &device_paths);
uint64_t get_used_space_bytes(const std::string &mount_point);
uint64_t get_disk_size_bytes(const std::string &mount_point);

