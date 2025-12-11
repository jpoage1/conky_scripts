cmake_minimum_required(VERSION 3.16)
project(ConkyWidgets VERSION 1.0 LANGUAGES CXX)

# --- Configuration ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# --- Dependencies ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
pkg_check_modules(LIBSSH REQUIRED libssh)

find_package(Lua REQUIRED)

# --- Compiler Flags ---
add_compile_options(-Wall -Wextra -O3 -march=native)

# --- Includes ---
include_directories(
    includes
    src/systeminfo
    src/cli
    src/processinfo
    src/data
    src/widgets
)

# --- Library: Telemetry Core ---
# Merges systeminfo, data, and runners to resolve circular dependencies
add_library(telemetry_core STATIC
    # Data & Streams
    src/data/data.cpp
    src/data/data_local.cpp
    src/data/data_ssh.cpp
    src/data/metrics.cpp
    src/ssh.cpp
    
    # System Info Logic
    src/systeminfo/corestat.cpp
    src/systeminfo/cpuinfo.cpp
    src/systeminfo/meminfo.cpp
    src/systeminfo/networkstats.cpp
    src/systeminfo/processinfo.cpp
    src/systeminfo/load_avg.cpp
    src/systeminfo/uptime.cpp
    src/systeminfo/diskstat.cpp
    src/systeminfo/filesystems.cpp
    src/systeminfo/sysinfo.cpp
    src/systeminfo/hwmonitor.cpp
    
    # Runners & Parsers
    src/lua_parser.cpp
    src/parser.cpp
    src/runner.cpp
    src/conky/conky_format.cpp
    src/conky/conky_output.cpp
)

# Link external deps to the core library
target_link_libraries(telemetry_core PUBLIC ${LIBSSH_LIBRARIES} stdc++fs ${LUA_LIBRARIES})
target_include_directories(telemetry_core PUBLIC ${LIBSSH_INCLUDE_DIRS} ${LUA_INCLUDE_DIR})

# --- Executable: Waybard ---
add_executable(waybard
    src/waybar/waybar_formatters.cpp
    src/waybar.cpp
)
target_link_libraries(waybard PRIVATE telemetry_core)

# --- Executable: Json ---
add_executable(json src/json.cpp)
target_link_libraries(json PRIVATE telemetry_core)

# --- Executable: Widgets ---
add_executable(widgets
    src/widgets/main.cpp
    src/widgets/widgets.cpp
    src/widgets/window.cpp
)
target_link_libraries(widgets PRIVATE telemetry_core ${GTKMM_LIBRARIES})
target_include_directories(widgets PRIVATE ${GTKMM_INCLUDE_DIRS})

# --- Install ---
install(TARGETS waybard json widgets RUNTIME DESTINATION bin)
install(TARGETS json COMPONENT json RUNTIME DESTINATION bin)
