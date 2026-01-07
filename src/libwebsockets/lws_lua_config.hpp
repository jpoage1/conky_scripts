#include "lua_generator.hpp"
#include <sstream>

namespace libwebsockets {

LuaConfigGenerator::LuaConfigGenerator() = default;

void LuaConfigGenerator::set_run_mode(const std::string &mode) {
  m_run_mode = mode;
}
void LuaConfigGenerator::set_output_format(const std::string &format) {
  m_output_format = format;
}

void LuaConfigGenerator::add_settings(const LuaSettings &s) {
  m_settings_list.push_back(s);
}

std::string LuaConfigGenerator::serialize_settings(const LuaSettings &s) const {
  std::ostringstream oss;
  oss << "    {\n";
  oss << "        name = \"" << s.name << "\",\n";
  oss << "        features = {\n";
  oss << "            enable_sysinfo = "
      << (s.features.enable_sysinfo ? "true" : "false") << ",\n";
  oss << "            processes = { count = " << s.features.process_count
      << " }\n";
  oss << "        },\n";
  oss << "        window = {\n";
  oss << "            x = " << s.window.x << ", y = " << s.window.y << ",\n";
  oss << "            visible = " << (s.window.visible ? "true" : "false")
      << "\n";
  oss << "        }\n";
  oss << "    }";
  return oss.str();
}

std::string LuaConfigGenerator::generate() const {
  std::ostringstream oss;
  oss << "config = {\n";
  oss << "    run_mode = \"" << m_run_mode << "\",\n";
  oss << "    output_format = \"" << m_output_format << "\",\n";
  oss << "    polling_interval_ms = " << m_interval_ms << ",\n";
  oss << "    settings = {\n";

  for (size_t i = 0; i < m_settings_list.size(); ++i) {
    oss << serialize_settings(m_settings_list[i]);
    if (i < m_settings_list.size() - 1)
      oss << ",";
    oss << "\n";
  }

  oss << "    }\n";
  oss << "}\n";
  oss << "return config";
  return oss.str();
}

} // namespace libwebsockets
