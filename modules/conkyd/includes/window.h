#pragma once

#include <gtkmm.h>

#include "data.h"
#include "widgets.h"

/**
 * @class ConkyWindow
 * @brief Main application window, lays out all widgets.
 *
 * This class corresponds to the 'conky' eww widget, which
 * arranges all other components.
 */
class ConkyWindow : public Gtk::Window {
 public:
  ConkyWindow();
  virtual ~ConkyWindow();

  /**
   * @brief Main update function called by the polling timer.
   *
   * @param metrics The system metrics from read_data().
   * @param disks The disk info gathered separately.
   */
  void update_all_data(const SystemMetrics& metrics,
                       const std::vector<DeviceInfo>& disks);

 protected:
  // Main layout boxes
  Gtk::Box m_main_vbox;
  Gtk::Box m_row1_hbox;
  Gtk::Box m_row2_hbox;

  // Widget components
  SysInfo m_sys_info;
  NetworkList m_network_list;
  MemSwap m_mem_swap;
  CpuCores m_cpu_cores;
  ProcList m_proc_cpu;
  ProcList m_proc_mem;
  DiskList m_disk_list;
};
