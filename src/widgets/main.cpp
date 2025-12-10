#include <gtkmm.h>
#include <gtkmm/cssprovider.h>
#include <stdlib.h>  // For setenv

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "data.hpp"
#include "data_local.hpp"
#include "filesystems.hpp"
#include "window.h"

// --- Config for mounts ---
// This would be your ~/.config/conky/file-systems.txt
const char* MOUNTS_CONFIG_FILE = "file-systems.txt";

// --- Data-gathering struct ---
// Holds all data needed by the UI
struct AllMetrics {
  SystemMetrics system;
  std::vector<DeviceInfo> disks;
};

// --- Polling function ---
// This function runs in the background
AllMetrics get_all_metrics(DataStreamProvider& provider,
                           const std::vector<std::string>& mounts) {
  AllMetrics all_data;

  // 1. Get SystemMetrics
  try {
    PollingTaskList polling_tasks = read_data(provider, all_data.system);
  } catch (const std::exception& e) {
    std::cerr << "Error reading system data: " << e.what() << std::endl;
  }

  // 2. Get Disk metrics
  for (const auto& mount_point : mounts) {
    try {
      DeviceInfo disk;
      disk.mount_point = mount_point;
      disk.size_bytes = provider.get_disk_size_bytes(mount_point);
      disk.used_bytes = provider.get_used_space_bytes(mount_point);
      all_data.disks.push_back(disk);
    } catch (const std::exception& e) {
      std::cerr << "Error reading disk data for " << mount_point << ": "
                << e.what() << std::endl;
    }
  }

  return all_data;
}
// --- Main Application Class ---
class ConkyApplication : public Gtk::Application {
 protected:  // MODIFIED: Constructor should be protected
  LocalDataStreams local_streams ConkyApplication()
      : Gtk::Application("org.conkyd.gtkmm"),
        m_provider(local_streams)  // Initialize local data provider
  {
    // Load mount points from config file
    std::ifstream ifs(MOUNTS_CONFIG_FILE);
    if (ifs.is_open()) {
      std::string line;
      while (std::getline(ifs, line)) {
        if (!line.empty() && line[0] != '#') {
          m_mount_points.push_back(line);
        }
      }
    } else {
      std::cerr << "Warning: Could not open " << MOUNTS_CONFIG_FILE
                << ". Disk widget will be empty." << std::endl;
      // Add defaults if you want
      // m_mount_points.push_back("/");
    }
  }

 public:  // ADDED: Public create() method
  static Glib::RefPtr<ConkyApplication> create() {
    return Glib::RefPtr<ConkyApplication>(new ConkyApplication());
  }

 protected:
  void on_activate() override {
    // Load CSS
    auto css_provider = Gtk::CssProvider::create();
    try {
      css_provider->load_from_path("style.css");
      Gtk::StyleContext::add_provider_for_display(
          Gdk::Display::get_default(), css_provider,
          GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    } catch (const Glib::Error& e) {  // Catch Glib::Error
      std::cerr << "Failed to load CSS: " << e.what() << std::endl;
    }

    // Create the main window
    m_window = new ConkyWindow();

    // Use std::static_pointer_cast
    auto app = std::static_pointer_cast<Gtk::Application>(
        Gtk::Application::get_default());
    m_window->set_application(app);

    m_window->show();

    // Initial data load
    poll_data();

    // Start polling timer (1000ms = 1s)
    Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &ConkyApplication::poll_data), 1000);
  }

  bool poll_data() {
    // This is where data is fetched and the UI is updated
    AllMetrics data = get_all_metrics(m_provider, m_mount_points);
    m_window->update_all_data(data.system, data.disks);
    return true;  // Keep timer running
  }

 private:
  ConkyWindow* m_window = nullptr;
  LocalDataStreams m_provider;
  std::vector<std::string> m_mount_points;
};

// --- Main Entry Point ---
int main(int argc, char* argv[]) {
  // FIX: Force OpenGL renderer
  setenv("GSK_RENDERER", "opengl", 1);

  // This now calls the static ConkyApplication::create()
  auto app = ConkyApplication::create();
  return app->run(argc, argv);
}
