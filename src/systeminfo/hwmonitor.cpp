// hwmonitor.cpp
#include "hwmonitor.hpp"

#include "data_local.hpp"
#include "provider.hpp"

double LocalDataStreams::get_cpu_temperature() {
  const std::string hwmon_base = "/sys/class/hwmon";
  std::optional<double> fallback_temp;  // Fallback to first core/sensor

  try {
    for (const auto& hwmon_dir :
         std::filesystem::directory_iterator(hwmon_base)) {
      if (!hwmon_dir.is_directory()) continue;

      auto name = read_sysfs_file(hwmon_dir.path() / "name");
      if (!name || (name.value() != "coretemp" && name.value() != "k10temp" &&
                    name.value() != "zenpower")) {
        continue;  // Not a known CPU sensor
      }

      for (const auto& file :
           std::filesystem::directory_iterator(hwmon_dir.path())) {
        std::string filename = file.path().filename().string();

        if (filename.rfind("temp", 0) == 0 &&
            filename.rfind("_input") != std::string::npos) {
          std::string label_filename = filename;
          label_filename.replace(label_filename.rfind("_input"), 6, "_label");

          auto label = read_sysfs_file(hwmon_dir.path() / label_filename);
          auto temp_str = read_sysfs_file(file.path());

          if (!temp_str) continue;

          double temp_celsius = std::stod(temp_str.value()) / 1000.0;

          if (label) {
            if (label.value() == "Tdie" || label.value() == "Package id 0") {
              return temp_celsius;  // Found package temp
            }
          }
          if (!fallback_temp) {
            fallback_temp = temp_celsius;
          }
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error reading temperature: " << e.what() << std::endl;
    return -1.0;
  }

  if (fallback_temp) {
    return fallback_temp.value();
  }

  return -1.0;  // Not found
}
