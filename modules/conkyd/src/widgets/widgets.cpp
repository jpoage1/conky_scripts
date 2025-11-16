#include "widgets.h"

#include <glibmm/ustring.h>

#include <iomanip>  // For std::setprecision
#include <sstream>  // For std::stringstream

// --- Helper to format numbers ---
template <typename T>
Glib::ustring format_num(T value, int precision = 1) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(precision) << value;
  return ss.str();
}

// --- SimpleBar ---
SimpleBar::SimpleBar(const Glib::ustring& label)
    : Gtk::Box(Gtk::Orientation::HORIZONTAL, 0),
      m_label(label),
      m_progress(),
      m_value_label("0%") {
  add_css_class("bar");
  m_label.add_css_class("bar-label");
  append(m_label);

  m_progress.set_hexpand(true);
  m_progress.set_show_text(false);
  append(m_progress);

  append(m_value_label);
}

void SimpleBar::update(double value, const Glib::ustring& text_value) {
  m_progress.set_fraction(value / 100.0);
  m_value_label.set_text(text_value);
}

// --- SysInfo ---
SysInfo::SysInfo()
    : Gtk::Frame("System"), m_box(Gtk::Orientation::VERTICAL, 5) {
  add_css_class("sys-info");
  m_box.set_margin(10);
  set_child(m_box);

  setup_label(m_label_host);
  m_box.append(m_label_host);

  setup_label(m_label_kernel);
  m_box.append(m_label_kernel);

  setup_label(m_label_uptime);
  m_box.append(m_label_uptime);

  setup_label(m_label_freq);
  m_box.append(m_label_freq);

  setup_label(m_label_temp);
  m_box.append(m_label_temp);

  setup_label(m_label_processes);
  m_box.append(m_label_processes);

  setup_label(m_label_load_avg);
  m_box.append(m_label_load_avg);

  update(SystemMetrics{});  // Initialize with empty data
}

void SysInfo::setup_label(Gtk::Label& label) {
  label.set_halign(Gtk::Align::START);
  label.set_selectable(true);
}

void SysInfo::update(const SystemMetrics& metrics) {
  m_label_host.set_text(
      Glib::ustring::sprintf("Host:   %s", metrics.node_name.c_str()));
  m_label_kernel.set_text(Glib::ustring::sprintf(
      "Kernel: %s %s %s", metrics.sys_name.c_str(),
      metrics.machine_type.c_str(), metrics.kernel_release.c_str()));
  m_label_uptime.set_text(
      Glib::ustring::sprintf("Uptime: %s", metrics.uptime.c_str()));
  m_label_freq.set_text(
      Glib::ustring::sprintf("Freq:   %.2f GHz", metrics.cpu_frequency_ghz));
  m_label_temp.set_text(
      Glib::ustring::sprintf("Temp:   %.1f°C", metrics.cpu_temp_c));
  m_label_processes.set_text(Glib::ustring::sprintf("Total: %ld | Running: %ld",
                                                    metrics.processes_total,
                                                    metrics.processes_running));
  m_label_load_avg.set_text(Glib::ustring::sprintf(
      "Load Average: 1m: %.2f | 5m: %.2f | 15m: %.2f", metrics.load_avg_1m,
      metrics.load_avg_5m, metrics.load_avg_15m));
}

// --- ProcList ---
ProcList::ProcList(const Glib::ustring& title)
    : Gtk::Frame(title), m_main_box(Gtk::Orientation::VERTICAL, 5) {
  add_css_class("proc-box");
  add_css_class("column");
  m_main_box.set_margin(10);
  set_child(m_main_box);

  // Header Row
  auto header_box =
      Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
  header_box->add_css_class("proc-header");
  header_box->set_hexpand(true);

  auto l_name = Gtk::make_managed<Gtk::Label>("Name");
  l_name->set_xalign(0);
  l_name->set_width_chars(20);  // eww width 140
  l_name->set_ellipsize(Pango::EllipsizeMode::END);
  header_box->append(*l_name);

  auto l_pid = Gtk::make_managed<Gtk::Label>("PID");
  l_pid->set_xalign(0);
  l_pid->set_width_chars(8);  // eww width 70
  header_box->append(*l_pid);

  auto l_cpu = Gtk::make_managed<Gtk::Label>("CPU");
  l_cpu->set_xalign(1.0);
  l_cpu->set_width_chars(8);  // eww width 70
  header_box->append(*l_cpu);

  auto l_mem = Gtk::make_managed<Gtk::Label>("MEM");
  l_mem->set_xalign(1.0);
  l_mem->set_width_chars(8);  // eww width 70
  header_box->append(*l_mem);

  m_main_box.append(*header_box);

  // Box to hold process rows
  m_list_box.set_orientation(Gtk::Orientation::VERTICAL);
  m_list_box.set_spacing(5);
  m_main_box.append(m_list_box);
}

void ProcList::update(const std::vector<ProcessInfo>& procs) {
  size_t num_procs = procs.size();

  // Ensure we have enough row widgets
  while (m_row_widgets.size() < num_procs) {
    auto row_box = std::make_unique<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
    row_box->set_hexpand(true);

    auto l_name = Gtk::make_managed<Gtk::Label>("");
    l_name->set_xalign(0);
    l_name->set_width_chars(20);
    l_name->set_ellipsize(Pango::EllipsizeMode::END);
    row_box->append(*l_name);

    auto l_pid = Gtk::make_managed<Gtk::Label>("");
    l_pid->set_xalign(0);
    l_pid->set_width_chars(8);
    row_box->append(*l_pid);

    auto l_cpu = Gtk::make_managed<Gtk::Label>("");
    l_cpu->set_xalign(1.0);
    l_cpu->set_width_chars(8);
    row_box->append(*l_cpu);

    auto l_mem = Gtk::make_managed<Gtk::Label>("");
    l_mem->set_xalign(1.0);
    l_mem->set_width_chars(8);
    row_box->append(*l_mem);

    m_list_box.append(*row_box);
    m_row_widgets.push_back(std::move(row_box));
  }

  // Update and show/hide rows
  for (size_t i = 0; i < m_row_widgets.size(); ++i) {
    auto* row_box = m_row_widgets[i].get();
    if (i < num_procs) {
      const auto& proc = procs[i];

      // Get children and cast them to Gtk::Label
      dynamic_cast<Gtk::Label*>(row_box->get_first_child())
          ->set_text(proc.name);

      auto* child = row_box->get_first_child()->get_next_sibling();
      dynamic_cast<Gtk::Label*>(child)->set_text(
          Glib::ustring::sprintf("%d", proc.pid));

      child = child->get_next_sibling();
      dynamic_cast<Gtk::Label*>(child)->set_text(format_num(proc.cpu_percent) +
                                                 "%");

      child = child->get_next_sibling();
      dynamic_cast<Gtk::Label*>(child)->set_text(
          format_num(proc.meminfo.percent) + "%");

      row_box->set_visible(true);
    } else {
      row_box->set_visible(false);
    }
  }
}

// --- NetworkList ---
NetworkList::NetworkList()
    : Gtk::Frame("Network (Raw Bytes/s)"),
      m_main_box(Gtk::Orientation::VERTICAL, 5) {
  set_halign(Gtk::Align::END);
  m_main_box.set_margin(10);
  set_child(m_main_box);

  auto header_box =
      Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
  header_box->add_css_class("proc-header");

  auto l_dev = Gtk::make_managed<Gtk::Label>("Device");
  l_dev->set_xalign(0);
  l_dev->set_width_chars(10);  // eww width 70
  header_box->append(*l_dev);

  auto l_down = Gtk::make_managed<Gtk::Label>("Down");
  l_down->set_xalign(1.0);
  l_down->set_width_chars(10);  // eww width 70
  header_box->append(*l_down);

  auto l_up = Gtk::make_managed<Gtk::Label>("Up");
  l_up->set_xalign(1.0);
  l_up->set_width_chars(10);  // eww width 70
  header_box->append(*l_up);

  m_main_box.append(*header_box);

  m_list_box.set_orientation(Gtk::Orientation::VERTICAL);
  m_list_box.set_spacing(5);
  m_main_box.append(m_list_box);
}

void NetworkList::update(const std::vector<NetworkInterfaceStats>& interfaces) {
  size_t num_ifaces = interfaces.size();

  while (m_row_widgets.size() < num_ifaces) {
    auto row_box = std::make_unique<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);

    auto l_dev = Gtk::make_managed<Gtk::Label>("");
    l_dev->set_xalign(0);
    l_dev->set_width_chars(10);
    row_box->append(*l_dev);

    auto l_down = Gtk::make_managed<Gtk::Label>("");
    l_down->set_xalign(1.0);
    l_down->set_width_chars(10);
    row_box->append(*l_down);

    auto l_up = Gtk::make_managed<Gtk::Label>("");
    l_up->set_xalign(1.0);
    l_up->set_width_chars(10);
    row_box->append(*l_up);

    m_list_box.append(*row_box);
    m_row_widgets.push_back(std::move(row_box));
  }

  for (size_t i = 0; i < m_row_widgets.size(); ++i) {
    auto* row_box = m_row_widgets[i].get();
    if (i < num_ifaces) {
      const auto& iface = interfaces[i];

      dynamic_cast<Gtk::Label*>(row_box->get_first_child())
          ->set_text(iface.interface_name);

      auto* child = row_box->get_first_child()->get_next_sibling();
      dynamic_cast<Gtk::Label*>(child)->set_text(
          format_num(iface.rx_bytes_per_sec));

      child = child->get_next_sibling();
      dynamic_cast<Gtk::Label*>(child)->set_text(
          format_num(iface.tx_bytes_per_sec));

      row_box->set_visible(true);
    } else {
      row_box->set_visible(false);
    }
  }
}

// --- DiskList ---
DiskList::DiskList()
    : Gtk::Frame("Disk Usage (GiB)"),
      m_main_box(Gtk::Orientation::VERTICAL, 5) {
  add_css_class("column");
  m_main_box.set_margin(10);
  set_child(m_main_box);

  auto header_box =
      Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
  header_box->add_css_class("proc-header");

  auto l_mount = Gtk::make_managed<Gtk::Label>("Mount");
  l_mount->set_xalign(0);
  l_mount->set_width_chars(20);  // eww width 150
  header_box->append(*l_mount);

  auto l_total = Gtk::make_managed<Gtk::Label>("Total");
  l_total->set_xalign(1.0);
  l_total->set_width_chars(8);  // eww width 60
  header_box->append(*l_total);

  auto l_used = Gtk::make_managed<Gtk::Label>("Used");
  l_used->set_xalign(1.0);
  l_used->set_width_chars(8);  // eww width 60
  header_box->append(*l_used);

  auto l_free = Gtk::make_managed<Gtk::Label>("Free");
  l_free->set_xalign(1.0);
  l_free->set_width_chars(8);  // eww width 60
  header_box->append(*l_free);

  auto l_pct = Gtk::make_managed<Gtk::Label>("Used %");
  l_pct->set_xalign(1.0);
  l_pct->set_width_chars(8);  // eww width 70
  header_box->append(*l_pct);

  m_main_box.append(*header_box);

  m_list_box.set_orientation(Gtk::Orientation::VERTICAL);
  m_list_box.set_spacing(5);
  m_main_box.append(m_list_box);
}

void DiskList::update(const std::vector<DeviceInfo>& disks) {
  const double GIB = 1073741824.0;
  size_t num_disks = disks.size();

  while (m_row_widgets.size() < num_disks) {
    auto row_box = std::make_unique<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);

    auto l_mount = Gtk::make_managed<Gtk::Label>("");
    l_mount->set_xalign(0);
    l_mount->set_width_chars(20);
    l_mount->set_ellipsize(Pango::EllipsizeMode::END);
    row_box->append(*l_mount);

    auto l_total = Gtk::make_managed<Gtk::Label>("");
    l_total->set_xalign(1.0);
    l_total->set_width_chars(8);
    row_box->append(*l_total);

    auto l_used = Gtk::make_managed<Gtk::Label>("");
    l_used->set_xalign(1.0);
    l_used->set_width_chars(8);
    row_box->append(*l_used);

    auto l_free = Gtk::make_managed<Gtk::Label>("");
    l_free->set_xalign(1.0);
    l_free->set_width_chars(8);
    row_box->append(*l_free);

    auto l_pct = Gtk::make_managed<Gtk::Label>("");
    l_pct->set_xalign(1.0);
    l_pct->set_width_chars(8);
    row_box->append(*l_pct);

    m_list_box.append(*row_box);
    m_row_widgets.push_back(std::move(row_box));
  }

  for (size_t i = 0; i < m_row_widgets.size(); ++i) {
    auto* row_box = m_row_widgets[i].get();
    if (i < num_disks) {
      const auto& disk = disks[i];

      double total_gib = disk.size_bytes / GIB;
      double used_gib = disk.used_bytes / GIB;
      double free_gib = (disk.size_bytes - disk.used_bytes) / GIB;
      double used_pct =
          (disk.size_bytes > 0)
              ? (static_cast<double>(disk.used_bytes) / disk.size_bytes) * 100.0
              : 0.0;

      auto* child = row_box->get_first_child();
      dynamic_cast<Gtk::Label*>(child)->set_text(disk.mount_point);

      child = child->get_next_sibling();
      dynamic_cast<Gtk::Label*>(child)->set_text(format_num(total_gib));

      child = child->get_next_sibling();
      dynamic_cast<Gtk::Label*>(child)->set_text(format_num(used_gib));

      child = child->get_next_sibling();
      dynamic_cast<Gtk::Label*>(child)->set_text(format_num(free_gib));

      child = child->get_next_sibling();
      dynamic_cast<Gtk::Label*>(child)->set_text(format_num(used_pct) + "%");

      row_box->set_visible(true);
    } else {
      row_box->set_visible(false);
    }
  }
}

// --- StackedCoreBar ---
StackedCoreBar::StackedCoreBar()
    : Gtk::Box(Gtk::Orientation::HORIZONTAL, 0),
      m_label_name("Avg"),
      m_label_total("0.0%"),
      m_bar_container(Gtk::Orientation::HORIZONTAL, 0),
      m_bar_user(),
      m_bar_system(),
      m_bar_nice(),
      m_bar_iowait(),
      m_bar_idle() {
  add_css_class("bar");

  m_label_name.add_css_class("bar-label-wide");
  append(m_label_name);

  m_bar_container.add_css_class("stacked-bar-container");
  m_bar_container.set_hexpand(true);

  m_bar_user.add_css_class("cpu-bar-user");
  m_bar_container.append(m_bar_user);

  m_bar_system.add_css_class("cpu-bar-system");
  m_bar_container.append(m_bar_system);

  m_bar_nice.add_css_class("cpu-bar-nice");
  m_bar_container.append(m_bar_nice);

  m_bar_iowait.add_css_class("cpu-bar-iowait");
  m_bar_container.append(m_bar_iowait);

  m_bar_idle.add_css_class("cpu-bar-idle");
  m_bar_container.append(m_bar_idle);

  append(m_bar_container);

  m_label_total.add_css_class("bar-total-label");
  append(m_label_total);
}

void StackedCoreBar::update(const CoreStats& core) {
  if (core.core_id == 0) {
    m_label_name.set_text("Avg");
  } else {
    m_label_name.set_text(Glib::ustring::sprintf("Core %zu", core.core_id - 1));
  }

  m_label_total.set_text(format_num(core.total_usage_percent) + "%");

  // Set bar sizes. Gtk::Box doesn't have a simple "width" property
  // like eww. We use set_size_request to set a width, but managing
  // percentages this way is tricky.
  // A better GTK4 way is Gtk::DrawingArea, but it's much more complex.
  // For simplicity, we'll use set_size_request.
  // This assumes the container width is ~100px.
  // A more robust way would be to use CSS.
  // For now, let's just show/hide and use hexpand for proportions.

  m_bar_user.set_tooltip_text(
      Glib::ustring::sprintf("User: %.1f%%", core.user_percent));
  m_bar_user.set_hexpand(core.user_percent > 0.1);

  m_bar_system.set_tooltip_text(
      Glib::ustring::sprintf("System: %.1f%%", core.system_percent));
  m_bar_system.set_hexpand(core.system_percent > 0.1);

  m_bar_nice.set_tooltip_text(
      Glib::ustring::sprintf("Nice: %.1f%%", core.nice_percent));
  m_bar_nice.set_hexpand(core.nice_percent > 0.1);

  m_bar_iowait.set_tooltip_text(
      Glib::ustring::sprintf("IOWait: %.1f%%", core.iowait_percent));
  m_bar_iowait.set_hexpand(core.iowait_percent > 0.1);

  m_bar_idle.set_tooltip_text(
      Glib::ustring::sprintf("Idle: %.1f%%", core.idle_percent));
  m_bar_idle.set_hexpand(core.idle_percent > 0.1);
}

// --- CpuCores ---
CpuCores::CpuCores()
    : Gtk::Frame("CPU"), m_main_box(Gtk::Orientation::VERTICAL, 2) {
  add_css_class("column");
  m_main_box.set_margin(10);
  set_child(m_main_box);

  // Header Row (omitted for brevity, eww config one is complex)

  m_list_box.set_orientation(Gtk::Orientation::VERTICAL);
  m_list_box.set_spacing(2);
  m_main_box.append(m_list_box);
}

void CpuCores::update(const std::vector<CoreStats>& cores) {
  size_t num_cores = cores.size();

  while (m_core_bars.size() < num_cores) {
    auto bar = std::make_unique<StackedCoreBar>();
    m_list_box.append(*bar);
    m_core_bars.push_back(std::move(bar));
  }

  for (size_t i = 0; i < m_core_bars.size(); ++i) {
    if (i < num_cores) {
      m_core_bars[i]->update(cores[i]);
      m_core_bars[i]->set_visible(true);
    } else {
      m_core_bars[i]->set_visible(false);
    }
  }
}

// --- MemSwap ---
MemSwap::MemSwap()
    : Gtk::Frame("Memory"),
      m_box(Gtk::Orientation::VERTICAL, 5),
      m_mem_bar("MEM"),
      m_swap_bar("SWP") {
  add_css_class("column");
  m_box.set_margin(10);
  set_child(m_box);

  m_box.append(m_mem_bar);
  m_label_mem_details.set_halign(Gtk::Align::START);
  m_box.append(m_label_mem_details);

  m_box.append(m_swap_bar);
  m_label_swap_details.set_halign(Gtk::Align::START);
  m_box.append(m_label_swap_details);

  update(SystemMetrics{});  // init
}

void MemSwap::update(const SystemMetrics& metrics) {
  m_mem_bar.update(metrics.meminfo.percent,
                   Glib::ustring::sprintf("%d%%", metrics.meminfo.percent));
  m_label_mem_details.set_text(Glib::ustring::sprintf(
      "Used: %ld MB / Total: %ld MB", metrics.meminfo.used_kb / 1024,
      metrics.meminfo.total_kb / 1024));

  m_swap_bar.update(metrics.swapinfo.percent,
                    Glib::ustring::sprintf("%d%%", metrics.swapinfo.percent));
  m_label_swap_details.set_text(Glib::ustring::sprintf(
      "Used: %ld MB / Total: %ld MB", metrics.swapinfo.used_kb / 1024,
      metrics.swapinfo.total_kb / 1024));
}
