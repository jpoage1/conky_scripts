#include "conky_output.hpp"

#include "conky_format.hpp"
#include "filesystems.hpp"
#include "format.hpp"
#include "processinfo.hpp"
#include "stream_provider.hpp"

std::unordered_map<size_t, int> column_widths = {
    {0, 20},  // column 0 has width 20
    {1, 20},  // column 1 has width 20
              // columns 2+ will use default
};

void print_metrics(const std::vector<DeviceInfo>& devices) {
  extern std::tuple<std::string, std::function<FuncType>> conky_columns[];
  extern const size_t CONKY_COLUMNS_COUNT;
  std::cout.setf(std::ios::unitbuf);

  print_column_headers(conky_columns, CONKY_COLUMNS_COUNT);
  print_rows(devices, CONKY_COLUMNS_COUNT);
}

void print_metrics(const SystemMetrics& metrics) {
  // Set precision for floating point numbers (percentages, temp, freq)
  std::cout << std::fixed << std::setprecision(1);

  std::cout << "CPU Frequency Ghz: " << metrics.cpu_frequency_ghz << std::endl;
  std::cout << "CPU Temp C: " << metrics.cpu_temp_c << " C" << std::endl;

  std::cout << "--- CPU Usage ---" << std::endl;
  // Loop over the vector of CoreStats (which now contains percentages)
  for (const auto& core : metrics.cores) {
    std::cout << "  Core " << std::setw(2) << core.core_id << ": "
              << std::setw(5) << core.total_usage_percent << "% "
              << "(User: " << std::setw(5) << core.user_percent << "%, "
              << "Sys: " << std::setw(5) << core.system_percent << "%, "
              << "IOWait: " << std::setw(5) << core.iowait_percent << "%)"
              << std::endl;
  }
  std::cout << "-----------------" << std::endl;

  std::cout << "Uptime: " << metrics.uptime << std::endl;

  std::cout << "Mem: " << metrics.meminfo.used_kb << " / "
            << metrics.meminfo.total_kb << " kB (" << metrics.meminfo.percent
            << "%)" << std::endl;

  std::cout << "Swap: " << metrics.swapinfo.used_kb << " / "
            << metrics.swapinfo.total_kb << " kB (" << metrics.swapinfo.percent
            << "%)" << std::endl;

  std::cout << "--- Top Processes (Mem) ---" << std::endl;
  std::cout << "PID\tVmRSS (MiB)\tName" << std::endl;
  for (const auto& proc : metrics.top_processes_avg_mem) {
    double vmRssMiB = static_cast<double>(proc.vmRssKb) / 1024.0;
    std::cout << proc.pid << "\t" << std::fixed << std::setprecision(1)
              << vmRssMiB << "\t\t" << proc.name << std::endl;
  }
  std::cout << "---------------------------" << std::endl;
  std::cout << "--- Top Processes (CPU) ---" << std::endl;
  std::cout << "PID\t%CPU\t\tName" << std::endl;
  // Iterate over the new vector, accessing the cpu_percent field
  for (const auto& proc : metrics.top_processes_avg_cpu) {
    std::cout << proc.pid << "\t" << std::fixed << std::setprecision(1)
              << proc.cpu_percent << "%\t\t" << proc.name << std::endl;
  }
  std::cout << "---------------------------" << std::endl;
}

void pad_str(const std::string& str) { std::cout << str; }

void pad_str(const ColoredString& cstr) {
  std::cout << "${color " << cstr.color << "}" << cstr.text << "${color}";
}
void print_column_headers(
    std::tuple<std::string, std::function<FuncType>> conky_columns[],
    size_t count) {
  std::cout << "${color " << colors::paleblue << "}";
  int xpos = 0;

  for (size_t i = 0; i < count; ++i) {
    std::cout << "${goto " << xpos << "}";
    pad_str(std::get<0>(conky_columns[i]));

    int width = DEFAULT_COL_WIDTH;
    if (column_widths.count(i)) width = column_widths[i];

    xpos += width * CHAR_WIDTH_PX;
  }

  std::cout << "${color}" << std::endl;
}

void print_rows(const std::vector<DeviceInfo>& devices,
                const size_t column_count) {
  extern std::tuple<std::string, std::function<FuncType>> conky_columns[];

  for (const auto& device : devices) {
    int xpos = 0;
    for (size_t i = 0; i < column_count; ++i) {
      std::cout << "${goto " << xpos << "}";
      pad_str(std::get<1>(conky_columns[i])(device));

      int width = DEFAULT_COL_WIDTH;
      if (column_widths.count(i)) width = column_widths[i];

      xpos += width * CHAR_WIDTH_PX;
    }
    std::cout << std::endl;
  }
}

int conky_diskstat(std::vector<DeviceInfo> devices) {
  extern std::tuple<std::string, std::function<FuncType>> conky_columns[];
  extern const size_t CONKY_COLUMNS_COUNT;

  std::cout.setf(std::ios::unitbuf);

  print_column_headers(conky_columns, CONKY_COLUMNS_COUNT);
  print_rows(devices, CONKY_COLUMNS_COUNT);
  return 0;
}
