#pragma once

#include <gtkmm.h>

#include "data.hpp"
#include "pcn.hpp"
struct Deviceinfo;
// A simple reusable progress bar (eww 'bar')
class SimpleBar : public Gtk::Box {
 public:
  SimpleBar(const Glib::ustring& label);
  void update(double value, const Glib::ustring& text_value);

 protected:
  Gtk::Label m_label;
  Gtk::ProgressBar m_progress;
  Gtk::Label m_value_label;
};

// eww 'sys-info'
class SysInfo : public Gtk::Frame {
 public:
  SysInfo();
  void update(const SystemMetrics& metrics);

 protected:
  void setup_label(Gtk::Label& label);

  Gtk::Box m_box;
  Gtk::Label m_label_host, m_label_kernel, m_label_uptime;
  Gtk::Label m_label_freq, m_label_temp, m_label_processes, m_label_load_avg;
};

// eww 'proc-list'
class ProcList : public Gtk::Frame {
 public:
  ProcList(const Glib::ustring& title);
  void update(const std::vector<ProcessInfo>& procs);

 protected:
  Gtk::Box m_main_box;
  Gtk::Box m_list_box;
  std::vector<std::unique_ptr<Gtk::Box>> m_row_widgets;
};

// eww 'network'
class NetworkList : public Gtk::Frame {
 public:
  NetworkList();
  void update(const std::vector<NetworkInterfaceStats>& interfaces);

 protected:
  Gtk::Box m_main_box;
  Gtk::Box m_list_box;
  std::vector<std::unique_ptr<Gtk::Box>> m_row_widgets;
};

// eww 'disk-list'
class DiskList : public Gtk::Frame {
 public:
  DiskList();
  void update(const std::vector<DeviceInfo>& disks);

 protected:
  Gtk::Box m_main_box;
  Gtk::Box m_list_box;
  std::vector<std::unique_ptr<Gtk::Box>> m_row_widgets;
};

// eww 'stacked-core-bar'
class StackedCoreBar : public Gtk::Box {
 public:
  StackedCoreBar();
  void update(const CoreStats& core);

 protected:
  Gtk::Label m_label_name;
  Gtk::Label m_label_total;
  Gtk::Box m_bar_container;
  Gtk::Box m_bar_user, m_bar_system, m_bar_nice, m_bar_iowait, m_bar_idle;
};

// eww 'cpu-cores'
class CpuCores : public Gtk::Frame {
 public:
  CpuCores();
  void update(const std::vector<CoreStats>& cores);

 protected:
  Gtk::Box m_main_box;
  Gtk::Box m_list_box;
  std::vector<std::unique_ptr<StackedCoreBar>> m_core_bars;
};

// eww 'mem-swap'
class MemSwap : public Gtk::Frame {
 public:
  MemSwap();
  void update(const SystemMetrics& metrics);

 protected:
  Gtk::Box m_box;
  SimpleBar m_mem_bar;
  Gtk::Label m_label_mem_details;
  SimpleBar m_swap_bar;
  Gtk::Label m_label_swap_details;
};
