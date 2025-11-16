// json_serialization.cpp
//
// 1. Include nlohmann/json ONCE.
#include "nlohmann/json.hpp"

// 2. Include all struct definitions.
#include "corestat.h"
#include "data.h"
#include "diskstat.hpp"
#include "json_definitions.hpp"
#include "networkstats.hpp"
#include "processinfo.hpp"
#include "runner.hpp"

// 3. Explicit inline functions for ProcessInfo
// These must be defined *after* nlohmann/json.hpp is included
inline void to_json(json& j, const ProcessInfo& p) {
  j = json{{"pid", p.pid},
           {"vmRssKb", p.vmRssKb},
           {"cpu_percent", p.cpu_percent},
           {"mem_percent", p.mem_percent},
           {"name", p.name}};
}

inline void from_json(const json& j, ProcessInfo& p) {
  j.at("pid").get_to(p.pid);
  j.at("vmRssKb").get_to(p.vmRssKb);
  j.at("cpu_percent").get_to(p.cpu_percent);
  j.at("mem_percent").get_to(p.mem_percent);
  j.at("name").get_to(p.name);
}
