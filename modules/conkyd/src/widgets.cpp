#include <glibmm/ustring.h>  // For Glib::ustring::sprintf
#include <gtkmm/application.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>

#include <iostream>
#include <string>

/**
 * @struct SystemData
 * @brief Holds all the system metrics, based on the eww config.
 *
 * This struct is assumed to be populated by another part of the
 * application and passed to the widget for display.
 */
struct SystemData {
  // Host: ${system-data[0].metrics.system.node_name}
  std::string node_name;

  // Kernel: ${...}.sys_name} ${...}.machine_type} ${...}.kernel_release}
  std::string sys_name;
  std::string machine_type;
  std::string kernel_release;

  // Uptime: ${...}.uptime}
  std::string uptime;  // Kept as string as it's often pre-formatted (e.g., "2
                       // days, 3:05")

  // Freq: ${...}.cpu_frequency_ghz}
  double cpu_frequency_ghz = 0.0;

  // Temp: ${...}.cpu_temp_c}
  double cpu_temp_c = 0.0;

  // Total: ${...}.processes_total} | Running: ${...}.processes_running}
  int processes_total = 0;
  int processes_running = 0;

  // Load Average: 1m: ${...}.load_avg_1m} | 5m: ${...}.load_avg_5m} | 15m:
  // ${...}.load_avg_15m}
  double load_avg_1m = 0.0;
  double load_avg_5m = 0.0;
  double load_avg_15m = 0.0;
};
/**
 * @class SysInfoWidget
 * @brief A static GTK widget to display system information.
 *
 * This widget translates the provided eww configuration into a C++/GTKmm
 * component. It inherits from Gtk::Frame to provide the "System" header.
 * The layout is static, but the data is dynamic and can be updated
 * via the update_data() method.
 */
class SysInfoWidget : public Gtk::Frame {
 public:
  /**
   * @brief Constructs the system information widget.
   */
  SysInfoWidget();
  virtual ~SysInfoWidget();

  /**
   * @brief Updates all labels in the widget with new data.
   * @param data The SystemData struct containing the new metrics.
   */
  void update_data(const SystemData& data);

 protected:
  // Child Widgets
  Gtk::Box m_main_box;

  Gtk::Label m_label_host;
  Gtk::Label m_label_kernel;
  Gtk::Label m_label_uptime;
  Gtk::Label m_label_freq;
  Gtk::Label m_label_temp;
  Gtk::Label m_label_processes;
  Gtk::Label m_label_load_avg;

 private:
  /**
   * @brief Helper function to initialize a label.
   * Sets horizontal alignment to START and selectable to true.
   */
  void setup_label(Gtk::Label& label);
};

SysInfoWidget::SysInfoWidget()
    : Gtk::Frame("System"),  // Use Frame label as the "header"
      m_main_box(Gtk::Orientation::VERTICAL, 5)  // spacing = 5
{
  // Configure the main box
  m_main_box.set_margin(10);
  set_child(m_main_box);

  // Initialize and pack all labels
  setup_label(m_label_host);
  m_main_box.append(m_label_host);

  setup_label(m_label_kernel);
  m_main_box.append(m_label_kernel);

  setup_label(m_label_uptime);
  m_main_box.append(m_label_uptime);

  setup_label(m_label_freq);
  m_main_box.append(m_label_freq);

  setup_label(m_label_temp);
  m_main_box.append(m_label_temp);

  setup_label(m_label_processes);
  m_main_box.append(m_label_processes);

  setup_label(m_label_load_avg);
  m_main_box.append(m_label_load_avg);

  // Initialize with empty data
  update_data(SystemData{});
}

SysInfoWidget::~SysInfoWidget() {}

void SysInfoWidget::setup_label(Gtk::Label& label) {
  // :halign "start" or :xalign 0
  label.set_halign(Gtk::Align::START);
  // Make text selectable for copy-pasting
  label.set_selectable(true);
}

void SysInfoWidget::update_data(const SystemData& data) {
  // Host:   ${...}.node_name}
  m_label_host.set_text(
      Glib::ustring::sprintf("Host:   %s", data.node_name.c_str()));

  // Kernel: ${...}.sys_name} ${...}.machine_type} ${...}.kernel_release}
  m_label_kernel.set_text(Glib::ustring::sprintf(
      "Kernel: %s %s %s", data.sys_name.c_str(), data.machine_type.c_str(),
      data.kernel_release.c_str()));

  // Uptime: ${...}.uptime}
  m_label_uptime.set_text(
      Glib::ustring::sprintf("Uptime: %s", data.uptime.c_str()));

  // Freq:   ${round(..., 2)} GHz
  m_label_freq.set_text(
      Glib::ustring::sprintf("Freq:   %.2f GHz", data.cpu_frequency_ghz));

  // Temp:   ${round(..., 1)}°C
  m_label_temp.set_text(
      Glib::ustring::sprintf("Temp:   %.1f°C", data.cpu_temp_c));

  // Total: ${...} | Running: ${...}
  m_label_processes.set_text(Glib::ustring::sprintf(
      "Total: %d | Running: %d", data.processes_total, data.processes_running));

  // Load Average: 1m: ${...} | 5m: ${...} | 15m: ${...}
  m_label_load_avg.set_text(Glib::ustring::sprintf(
      "Load Average: 1m: %.2f | 5m: %.2f | 15m: %.2f", data.load_avg_1m,
      data.load_avg_5m, data.load_avg_15m));
}

// Forward declaration for the main window
class DemoWindow : public Gtk::Window {
 public:
  DemoWindow();
  virtual ~DemoWindow();

 protected:
  SysInfoWidget m_sys_widget;
};

DemoWindow::DemoWindow() {
  set_title("System Info Widget Demo");
  set_default_size(300, 200);

  // Set the widget as the window's child
  set_child(m_sys_widget);

  // --- Create dummy data to populate the widget ---
  // In a real application, you would get this data from
  // system calls, /proc, or another service.
  SystemData dummy_data;
  dummy_data.node_name = "nixos-desktop";  // From user profile
  dummy_data.sys_name = "Linux";
  dummy_data.machine_type = "x86_64";
  dummy_data.kernel_release = "6.5.0-nixos";
  dummy_data.uptime = "3 days, 14:25";
  dummy_data.cpu_frequency_ghz = 3.4567;
  dummy_data.cpu_temp_c = 45.8;
  dummy_data.processes_total = 345;
  dummy_data.processes_running = 2;
  dummy_data.load_avg_1m = 0.75;
  dummy_data.load_avg_5m = 0.52;
  dummy_data.load_avg_15m = 0.48;

  // Update the widget with the dummy data
  m_sys_widget.update_data(dummy_data);
}

DemoWindow::~DemoWindow() {}

int main(int argc, char* argv[]) {
  setenv("GSK_RENDERER", "opengl", 1);

  // Use gtkmm-4.0
  auto app = Gtk::Application::create("org.gtkmm.example.syswidget");

  // Show the window and run the application
  return app->make_window_and_run<DemoWindow>(argc, argv);
}
