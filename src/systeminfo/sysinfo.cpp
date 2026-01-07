#include "sysinfo.hpp"

#include <sys/utsname.h>

#include "metrics.hpp"
#include "stream_provider.hpp"

namespace telemetry {

// Helper to read a single line from a file
std::string read_line(const std::string &path) {
  std::ifstream f(path);
  std::string line;
  if (std::getline(f, line)) {
    // Remove trailing newline if present
    if (!line.empty() && line.back() == '\n')
      line.pop_back();
    return line;
  }
  return "";
}
std::string get_real_machine_type() {
  struct utsname uts_info;
  if (uname(&uts_info) == 0) {
    return uts_info.machine;
  }
  return "Unknown";
}
void get_system_info(SystemMetrics &metrics) {
  // 1. Try to read from standard kernel interfaces (VFS-friendly)
  std::string hostname = read_line("/proc/sys/kernel/hostname");
  std::string release = read_line("/proc/sys/kernel/osrelease");
  std::string ostype = read_line("/proc/sys/kernel/ostype");
  std::string machine = read_line("/proc/sys/kernel/machine");

  // Some systems expose architecture in /proc/sys/kernel/arch, but it's less
  // standard. We can rely on uname for machine type unless we want to mock a
  // specific file.

  if (!hostname.empty() && !release.empty() && !ostype.empty()) {
    metrics.node_name = hostname;
    metrics.kernel_release = release;
    metrics.sys_name = ostype;
    metrics.machine_type = machine.empty() ? get_real_machine_type() : machine;
  } else {
    // 2. Fallback to syscall (Real Hardware behavior)
    struct utsname uts_info;
    if (uname(&uts_info) == 0) {
      metrics.sys_name = uts_info.sysname;
      metrics.node_name = uts_info.nodename;
      metrics.kernel_release = uts_info.release;
      metrics.machine_type = uts_info.machine;
    } else {
      metrics.sys_name = "Unknown";
      metrics.node_name = "Unknown";
      metrics.kernel_release = "Unknown";
      metrics.machine_type = "Unknown";
    }
  }
}
}; // namespace telemetry
