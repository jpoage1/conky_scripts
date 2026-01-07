// batteryinfo.hpp
#ifndef BATTERYINFO_HPP
#define BATTERYINFO_HPP

#include "pcn.hpp"
namespace telemetry {
namespace fs = std::filesystem;
// Runtime Data (from /sys)
struct BatteryStatus {
  std::string name;
  int percentage = 0;
  std::string status = "Unknown"; // Charging, Discharging, Full
  std::string path;
  bool present = false;
};

struct Battery {
  class BatteryStatus {
  public:
    enum Value { CHARGING, DISCHARGING, FULL, UNKNOWN };

    BatteryStatus(Value v = UNKNOWN);

    operator Value() const;

    static std::string get_str(Value v);
    const std::string get_str() const;
    void set(const std::string &val);

  private:
    Value m_val;
  }; // End BatteryStatus

  std::string name = "Battery";
  std::string path = "/sys/class/power_supply/BAT0";
  int critical_threshold = 15;
  std::string icon = "⚡";
  BatteryStatus status = BatteryStatus::UNKNOWN;

  Battery(std::string p);
  Battery(std::string n, std::string p);
  ~Battery() = default;

}; // end Battery

// Batteries
struct Batteries {
  Batteries();
  ~Batteries() = default;

  using iterator = std::vector<Battery>::iterator;
  using const_iterator = std::vector<Battery>::const_iterator;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;
  size_t size() const;
  bool empty() const;
  const Battery &operator[](size_t i) const;

private:
  std::vector<Battery> m_list;
}; // End Batteries

class LuaBatteryStatus {
  std::string serialize(unsigned indentation_level = 0);
};

// Battery
struct LuaBattery : public Battery {
  std::string serialize(unsigned indentation_level = 0) const;
  void deserialize(const sol::table &bat_item);
}; // end Battery

// Batteries
struct LuaBatteries : public Batteries {
  std::string serialize(unsigned indentation_level = 0) const;

  void deserialize(sol::table batteries_table);
}; // End Batteries

}; // namespace telemetry
#endif
