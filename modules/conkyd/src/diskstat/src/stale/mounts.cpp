#include <limits.h>
#include <sys/statvfs.h>

#include <chrono>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

/*************
 * Constants *
 * ***********/

const int COL_WIDTH = 20;
const int COL_WIDTH_1 = 30;

const std::string paleblue = "00ccff";
const std::string lightgrey = "lightgrey";
const std::string palerblue = "00eeff";
const std::string red = "ff0000";
const std::string yellow = "ffcc00";
const std::string green = "33cc33";
const std::string reset = "";  // set to some color or will break
const std::string gray = "999999";
const std::string blue = "3399ff";

/***********
 * Structs *
 * ********/

struct DeviceInfo {
  std::string device_path;
  std::string mount_point;
  uint64_t used_bytes = 0;
  uint64_t size_bytes = 0;
  std::string used_space;
  std::string size;
  std::string used_space_percent;
  std::string read_bytes_per_sec;
  std::string write_bytes_per_sec;
};

struct DeviceStats {
  uint64_t prev_bytes_read = 0;
  uint64_t prev_bytes_written = 0;
  uint64_t prev_timestamp_ms = 0;
};

struct ColoredString {
  std::string text;   // Unformatted text for padding
  std::string color;  // Conky color tag, e.g., "lightgrey"

  std::string formatted() const {
    return "${color " + color + "}" + text + "${color}";
  }

  explicit ColoredString(const std::string &t, const std::string &c)
      : text(t), color(c) {}
};

std::unordered_map<std::string, DeviceStats> stats_map;

using FuncType = ColoredString(const DeviceInfo &);

/***********************
 * Function Signatures *
 * *********************/
std::string get_mount_point(const std::string &);
uint64_t get_used_space_bytes(const std::string &);
uint64_t get_disk_size_bytes(const std::string &);
std::string get_read_bytes_per_sec(const std::string &);
std::string get_write_bytes_per_sec(const std::string &);

void print_rows(std::vector<DeviceInfo> &, size_t);
void print_column_headers(std::tuple<std::string, std::function<FuncType>>[],
                          size_t count);
void pad_str(const ColoredString &, int);
void pad_str(const std::string &, int);

std::vector<DeviceInfo> collect_device_info(const std::vector<std::string> &);

ColoredString conky_color(const std::string &, const std::string &);
ColoredString col_device(const DeviceInfo &);
ColoredString col_mount(const DeviceInfo &);
ColoredString col_used(const DeviceInfo &);
ColoredString col_size(const DeviceInfo &);
ColoredString col_used_space_percent(const DeviceInfo &);
ColoredString col_read_bytes_per_sec(const DeviceInfo &);
ColoredString col_write_bytes_per_sec(const DeviceInfo &);
ColoredString format_size(uint64_t bytes);
/*******************
 *                 *
 * *****************/
std::tuple<std::string, std::function<ColoredString(const DeviceInfo &)>>
    columns[] = {

        {"Device", col_device},
        {"Mount Point", col_mount},
        {"Used", col_used},
        {"Size", col_size},
        {"Use%", col_used_space_percent},
        {"Read/s", col_read_bytes_per_sec},
        {"Write/s", col_write_bytes_per_sec}};

ColoredString col_device(const DeviceInfo &d) {
  return conky_color(d.device_path, palerblue);
}

ColoredString col_mount(const DeviceInfo &d) {
  return conky_color(d.mount_point, lightgrey);
}

ColoredString col_used(const DeviceInfo &d) {
  return format_size(d.used_bytes);
}

ColoredString col_size(const DeviceInfo &d) {
  return format_size(d.size_bytes);
}

ColoredString col_used_space_percent(const DeviceInfo &d) {
  std::string color = green;
  if (d.size_bytes == 0) return conky_color("N/A", gray);

  uint64_t percent = (d.used_bytes * 100) / d.size_bytes;

  if (percent >= 90)
    color = red;
  else if (percent >= 75)
    color = yellow;
  else
    color = green;
  return conky_color(std::to_string(percent) + "%", color);
}

ColoredString col_read_bytes_per_sec(const DeviceInfo &d) {
  const std::string rate = d.read_bytes_per_sec.empty()
                               ? "${diskio_read(" + d.device_path + ")}"
                               : d.read_bytes_per_sec;
  const std::string color = rate == "N/A" ? gray : blue;
  return conky_color(rate, color);
}

ColoredString col_write_bytes_per_sec(const DeviceInfo &d) {
  const std::string rate = d.write_bytes_per_sec.empty()
                               ? "${diskio_write(" + d.device_path + ")}"
                               : d.write_bytes_per_sec;
  const std::string color = rate == "N/A" ? gray : blue;
  return conky_color(rate, color);
}

std::vector<std::string> read_device_paths(const std::string &path) {
  std::vector<std::string> paths;
  std::ifstream file(path);
  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty()) {
      paths.push_back(line);
    }
  }
  return paths;
}

/********
 * Main *
 * ******/
int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <device_list_file>\n";
    return 1;
  }

  std::vector<std::string> device_paths = read_device_paths(argv[1]);
  std::vector<DeviceInfo> devices = collect_device_info(device_paths);
  size_t column_count = sizeof(columns) / sizeof(columns[0]);
  print_column_headers(columns, column_count);
  print_rows(devices, column_count);
  return 0;
}

/***********************
 * Collect Device info *
 * *********************/

std::vector<DeviceInfo> collect_device_info(
    const std::vector<std::string> &device_paths) {
  std::vector<DeviceInfo> data;
  for (const auto &device_path : device_paths) {
    DeviceInfo info;
    info.device_path = device_path;
    info.mount_point = get_mount_point(device_path);
    info.used_bytes = get_used_space_bytes(info.mount_point);
    info.size_bytes = get_disk_size_bytes(info.mount_point);
    info.read_bytes_per_sec = get_read_bytes_per_sec(device_path);
    info.write_bytes_per_sec = get_write_bytes_per_sec(device_path);
    data.push_back(info);
  }
  return data;
}

/**************
 * Print Rows *
 * ************/

void print_rows(std::vector<DeviceInfo> &devices, size_t column_count) {
  for (const auto &device : devices) {
    for (size_t i = 0; i < column_count; ++i) {
      int col_width = i == 0 ? COL_WIDTH_1 : COL_WIDTH;
      auto &fn = std::get<1>(columns[i]);

      ColoredString str = fn ? fn(device) : conky_color("[N/A]", "gray");

      pad_str(str, col_width);
    }
    std::cout << std::endl;
  }
}
void pad_str(const ColoredString &cstr, int width) {
  std::ostringstream ss;
  ss << std::left << std::setw(width) << cstr.text;

  std::string color = cstr.color;

  std::cout << "${color " << color << "}" << ss.str() << "${color}";
}

void pad_str(const std::string &str, int width) {
  std::cout << std::left << std::setw(width) << str;
}
/************************q
 * Print Column Headers *
 * **********************/

void print_column_headers(
    std::tuple<std::string, std::function<FuncType>> columns[], size_t count) {
  std::cout << "${color " << paleblue << "}";
  for (size_t i = 0; i < count; ++i) {
    int col_width = i == 0 ? COL_WIDTH_1 : COL_WIDTH;
    auto header = std::get<0>(columns[i]);
    pad_str(header, col_width);
  }
  std::cout << "${color " << paleblue << "}" << std::endl;
}

/*******************
 * Get Mount Point *
 * *****************/
std::string get_mount_point(const std::string &device_path) {
  std::ifstream mounts("/proc/mounts");
  std::string device, mount_point, rest;

  while (mounts >> device >> mount_point) {
    std::getline(mounts, rest);

    char dev_real[PATH_MAX], path_real[PATH_MAX];
    if (realpath(device.c_str(), dev_real) &&
        realpath(device_path.c_str(), path_real)) {
      if (strcmp(dev_real, path_real) == 0) {
        return mount_point;
      }
    }
  }
  return "";
}

/******************
 * Get Used Space *
 * ****************/

uint64_t get_used_space_bytes(const std::string &mount_point) {
  if (mount_point.empty()) return 0;
  struct statvfs stat;
  if (statvfs(mount_point.c_str(), &stat) != 0) return 0;
  return (stat.f_blocks - stat.f_bfree) * stat.f_frsize;
}

/*****************
 * Get Disk Size *
 * ***************/

uint64_t get_disk_size_bytes(const std::string &mount_point) {
  if (mount_point.empty()) return 0;
  struct statvfs stat;
  if (statvfs(mount_point.c_str(), &stat) != 0) return 0;
  return stat.f_blocks * stat.f_frsize;
}
uint64_t current_time_in_ms() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
      .count();
}

std::string get_read_bytes_per_sec(const std::string &device_path) {
  if (device_path.empty()) return "";

  // Extract device basename and strip partition digits
  auto pos = device_path.find_last_of('/');
  if (pos == std::string::npos) return "";

  std::string device_name = device_path.substr(pos + 1);
  while (!device_name.empty() && isdigit(device_name.back())) {
    device_name.pop_back();
  }

  // Read /proc/diskstats for sectors read
  uint64_t current_bytes_read = 0;
  std::ifstream diskstats("/proc/diskstats");
  if (!diskstats.is_open()) return "";

  std::string line;
  while (std::getline(diskstats, line)) {
    std::istringstream iss(line);
    std::string field;
    int field_index = 0;
    std::string dev_name;
    uint64_t sectors_read = 0;

    while (iss >> field) {
      ++field_index;
      if (field_index == 3)
        dev_name = field;
      else if (field_index == 6)
        sectors_read = std::stoull(field);
    }

    if (dev_name == device_name) {
      current_bytes_read = sectors_read * 512;  // sector size 512 bytes
      break;
    }
  }

  uint64_t now_ms = current_time_in_ms();
  auto &stat = stats_map[device_name];

  uint64_t delta_bytes = 0;
  uint64_t delta_ms = 1;  // prevent div by zero

  if (stat.prev_timestamp_ms != 0 && now_ms > stat.prev_timestamp_ms) {
    delta_bytes = current_bytes_read - stat.prev_bytes_read;
    delta_ms = now_ms - stat.prev_timestamp_ms;
  }

  stat.prev_bytes_read = current_bytes_read;
  stat.prev_timestamp_ms = now_ms;

  uint64_t bytes_per_sec = (delta_bytes * 1000) / delta_ms;

  return format_size(bytes_per_sec).text;
}
std::string get_write_bytes_per_sec(const std::string &device_path) {
  if (device_path.empty()) return "";

  auto pos = device_path.find_last_of('/');
  if (pos == std::string::npos) return "";

  std::string device_name = device_path.substr(pos + 1);
  while (!device_name.empty() && isdigit(device_name.back())) {
    device_name.pop_back();
  }
  if (device_name.empty()) return "";

  uint64_t current_bytes_written = 0;
  std::ifstream diskstats("/proc/diskstats");
  if (!diskstats.is_open()) return "";

  std::string line;
  while (std::getline(diskstats, line)) {
    std::istringstream iss(line);
    std::string field;
    int field_index = 0;
    std::string dev_name;
    uint64_t sectors_written = 0;

    while (iss >> field) {
      ++field_index;
      if (field_index == 3)
        dev_name = field;
      else if (field_index == 10)
        sectors_written = std::stoull(field);
    }

    if (dev_name == device_name) {
      current_bytes_written = sectors_written * 512;
      break;
    }
  }

  uint64_t now_ms = current_time_in_ms();
  auto &stat = stats_map[device_name];

  uint64_t delta_bytes = 0;
  uint64_t delta_ms = 1;

  if (stat.prev_timestamp_ms != 0 && now_ms > stat.prev_timestamp_ms) {
    delta_bytes = current_bytes_written - stat.prev_bytes_written;
    delta_ms = now_ms - stat.prev_timestamp_ms;
  }

  stat.prev_bytes_written = current_bytes_written;
  stat.prev_timestamp_ms = now_ms;

  uint64_t bytes_per_sec = (delta_bytes * 1000) / delta_ms;

  return format_size(bytes_per_sec).text;
}
ColoredString format_size(uint64_t bytes) {
  constexpr uint64_t KB = 1024;
  constexpr uint64_t MB = KB * 1024;
  constexpr uint64_t GB = MB * 1024;

  std::string size;

  if (bytes >= GB) {
    size = std::to_string(bytes / GB) + "G";
    return conky_color(size, "00ff00");
  } else if (bytes >= MB) {
    size = std::to_string(bytes / MB) + "M";
    return conky_color(size, "ffaa00");
  } else if (bytes >= KB) {
    size = std::to_string(bytes / KB) + "K";
    return conky_color(size, red);
  } else if (bytes > 0) {
    size = std::to_string(bytes) + "B";
    return conky_color(size, red);
  } else {
    return conky_color("N/A", gray);
  }
}

ColoredString conky_color(const std::string &text,
                          const std::string &color = lightgrey) {
  return ColoredString{text, color};
}
