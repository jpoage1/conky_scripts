// batteryinfo.hpp
#ifndef BATTERYINFO_HPP
#define BATTERYINFO_HPP

#include "pcn.hpp"
namespace telemetry {
namespace fs = std::filesystem;

class BatteryStatus {
public:
  std::string name;
  int percentage = 0;
  std::string status = "Unknown"; // Charging, Discharging, Full
  std::string path;
  bool present = false;
  /* ^^^ LEGACY LOGIC ^^^ */
  enum Value { CHARGING, DISCHARGING, FULL, UNKNOWN };

  BatteryStatus(Value v = UNKNOWN);

  operator Value() const { return m_val; };

  static std::string get_str(Value v);
  const std::string get_str() const;
  void set(const std::string &val);

private:
  Value m_val;
}; // End BatteryStatus
struct Battery {

  std::string name = "Battery";
  std::string path = "/sys/class/power_supply/BAT0";
  int critical_threshold = 15;
  std::string icon = "⚡";
  BatteryStatus status = BatteryStatus::UNKNOWN;

  Battery(std::string path); // fixme, verify battery path exists
  Battery(std::string name, std::string path);
  ~Battery() = default;

}; // end Battery

// Batteries
struct Batteries {
  Batteries();
  ~Batteries() = default;

  using iterator = std::vector<Battery>::iterator;
  using const_iterator = std::vector<Battery>::const_iterator;

  iterator begin() { return m_list.begin(); }
  iterator end() { return m_list.end(); }
  const_iterator begin() const { return m_list.begin(); }
  const_iterator end() const { return m_list.end(); }
  size_t size() const;
  bool empty() const;
  const Battery &operator[](size_t i) const { return m_list[i]; };

protected:
  std::vector<Battery> m_list;
}; // End Batteries

class LuaBatteryStatus : public BatteryStatus {
  std::string serialize(unsigned int indentation_level = 0) const;
};

// Battery
struct LuaBattery : public Battery {
  std::string serialize(unsigned int indentation_level = 0) const;
  static std::string serialize(const Battery &self,
                               unsigned int indentation_level = 0);
  void deserialize(const sol::table &bat_item);
  static void deserialize(Battery &self, const sol::table &bat_item);
}; // end Battery

// Batteries
struct LuaBatteries : public Batteries {
  std::string serialize(unsigned int indentation_level = 0) const;

  void deserialize(sol::table batteries_table);
}; // End Batteries

}; // namespace telemetry
#endif
