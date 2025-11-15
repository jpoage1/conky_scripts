#include "sysinfo.hpp"

#include <sys/utsname.h>

void get_system_info(SystemMetrics& metrics) {
  struct utsname uts_info;
  if (uname(&uts_info) == 0) {  // 0 indicates success
    metrics.sys_name = uts_info.sysname;
    metrics.node_name = uts_info.nodename;
    metrics.kernel_release = uts_info.release;
    metrics.machine_type = uts_info.machine;
  } else {
    // Handle uname error if needed, e.g., set default strings
    metrics.sys_name = "N/A";
    metrics.node_name = "N/A";
    metrics.kernel_release = "N/A";
    metrics.machine_type = "N/A";
  }
}
