#include <istream>
#include <sstream>
#include <string>

#include "data.h"
#include "ssh.h"

struct ProcDataStreams : public DataStreamProvider {
  std::stringstream cpuinfo;
  std::stringstream meminfo;
  std::stringstream uptime;
  std::stringstream stat;
  std::stringstream mounts;
  std::stringstream diskstats;
  std::stringstream loadavg;
  std::stringstream net_dev;
  std::stringstream top_mem_procs;
  std::stringstream top_cpu_procs;

  ProcDataStreams () {
        std::string cpu_data = execute_ssh_command("cat /proc/cpuinfo");
        std::string meminfo_data = execute_ssh_command("cat /proc/meminfo");
        std::string uptime_data = execute_ssh_command("cat /proc/uptime");
        std::string stat_data = execute_ssh_command("cat /proc/stat");
        std::string mounts_data = execute_ssh_command("cat /proc/mounts");
        std::string diskstats_data = execute_ssh_command("cat /proc/diskstats");
        std::string loadavg_data = execute_ssh_command("cat /proc/loadavg");
        std::string net_dev_data = execute_ssh_command("cat /proc/net/dev");
        std::string top_mem_data = execute_ssh_command(
            "ps -eo pid,rss,comm --no-headers --sort=-rss | grep -v \" ps$\" | head "
            "-n 10");
        std::string top_cpu_data = execute_ssh_command(
            "ps -eo pid,%cpu,rss,comm --no-headers --sort=-%cpu | grep -v \" ps$\" | "
            "head -n 10");

    cpuinfo.str(cpu_data);
    meminfo.str(meminfo_data);
    uptime.str(uptime_data);
    stat.str(stat_data);
    mounts.str(mounts_data);
    diskstats.str(diskstats_data);
    loadavg.str(loadavg_data);
    net_dev.str(net_dev_data);
    top_mem_procs.str(top_mem_data);
    top_cpu_procs.str(top_cpu_data);
  }


  std::istream& get_cpuinfo_stream() override {
    rewind(cpuinfo, "cpuinfo");
    return cpuinfo;
  }
  std::istream& get_meminfo_stream() override {
    rewind(meminfo, "meminfo");
    return meminfo;
  }
  std::istream& get_uptime_stream() override {
    rewind(uptime, "uptime");
    return uptime;
  }
  std::istream& get_stat_stream() override {
    rewind(stat, "stat");
    return stat;
  }
  std::istream& get_mounts_stream() override {
    rewind(mounts, "mounts");
    return mounts;
  }
  std::istream& get_diskstats_stream() override {
    rewind(diskstats, "diskstats");
    return diskstats;
  }
  std::istream& get_loadavg_stream() override {
    rewind(loadavg, "loadavg");
    return loadavg;
  }
  std::istream& get_net_dev_stream() override {
    rewind(net_dev, "net_dev");
    return net_dev;
  }
  std::istream& get_top_mem_processes_stream() override {
    rewind(top_mem_procs, "top_mem_procs");
    return top_mem_procs;
  };
  std::istream& get_top_cpu_processes_stream() override {
    rewind(top_cpu_procs, "top_cpu_procs");
    return top_cpu_procs;
  }
  uint64_t get_used_space_bytes(const std::string& mount_point) override;
  uint64_t get_disk_size_bytes(const std::string& mount_point) override;

  double get_cpu_temperature() override {
    return -1.0;
  }
};


std::string trim(const std::string& str);
uint64_t get_df_data_bytes(const std::string& mount_point, bool get_used);

// uint64_t ProcDataStreams::get_used_space_bytes(const std::string
// &mount_point); uint64_t ProcDataStreams::get_disk_size_bytes(const
// std::string &mount_point);
