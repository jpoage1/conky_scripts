#include "window.h"

ConkyWindow::ConkyWindow()
    : m_main_vbox(Gtk::Orientation::VERTICAL, 15),
      m_row1_hbox(Gtk::Orientation::HORIZONTAL, 15),
      m_row2_hbox(Gtk::Orientation::HORIZONTAL, 15),
      m_proc_cpu("Top CPU"),
      m_proc_mem("Top Mem") {
  set_title("Conky GTKmm");
  set_default_size(700, 900);
  add_css_class("main-widget");

  m_main_vbox.set_margin(15);
  set_child(m_main_vbox);

  // TOP ROW: System Info, Network, and Memory/Swap
  // (eww box :orientation "horizontal")
  m_row1_hbox.set_homogeneous(true);  // eww :space-evenly true
  m_row1_hbox.append(m_sys_info);
  m_row1_hbox.append(m_network_list);
  m_row1_hbox.append(m_mem_swap);
  m_main_vbox.append(m_row1_hbox);

  // CPU Cores
  m_main_vbox.append(m_cpu_cores);

  // Middle Row: Top CPU and Top Mem
  // (eww box :orientation "horizontal")
  m_row2_hbox.set_homogeneous(true);  // eww :space-evenly true
  m_row2_hbox.append(m_proc_cpu);
  m_row2_hbox.append(m_proc_mem);
  m_main_vbox.append(m_row2_hbox);

  // Bottom Row: Disk List
  m_main_vbox.append(m_disk_list);
}

ConkyWindow::~ConkyWindow() {}

void ConkyWindow::update_all_data(const SystemMetrics& metrics,
                                  const std::vector<DeviceInfo>& disks) {
  // Dispatch updates to all child widgets
  m_sys_info.update(metrics);
  m_network_list.update(metrics.network_interfaces);
  m_mem_swap.update(metrics);
  m_cpu_cores.update(metrics.cores);
  m_proc_cpu.update(metrics.top_processes_avg_cpu);
  m_proc_mem.update(metrics.top_processes_avg_mem);
  m_disk_list.update(disks);
}
