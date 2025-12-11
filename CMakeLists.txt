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
find_package(spdlog REQUIRED)

# --- Compiler Flags ---
add_compile_options(-Wall -Wextra -O3 -march=native -fsanitize=address)
add_link_options(-fsanitize=address)

# --- Includes ---
include_directories(
    includes
    src/systeminfo
    src/cli
    src/processinfo
    src/data
    src/widgets
)
# =========================================================
# LIBRARY 1: Backend (Base Layer)
# Low-level data acquisition. No dependencies on logic.
# =========================================================
add_library(telemetry_backend STATIC
    src/data/data.cpp
    src/data/data_local.cpp
    src/data/data_ssh.cpp
    src/ssh.cpp
)
target_compile_definitions(telemetry_backend PUBLIC SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
target_link_libraries(telemetry_backend PUBLIC ${LIBSSH_LIBRARIES} stdc++fs spdlog::spdlog)
target_include_directories(telemetry_backend PUBLIC ${LIBSSH_INCLUDE_DIRS})

# =========================================================
# LIBRARY 2: Features (The Fix: Merged Logic Layer)
# Contains BOTH Metrics structs AND Polling Tasks to resolve the cycle.
# =========================================================
add_library(telemetry_features STATIC
    # Data Definitions
    src/data/metrics.cpp 
    src/data/polling.cpp

    # Logic & Tasks
    src/systeminfo/corestat.cpp
    src/systeminfo/networkstats.cpp
    src/systeminfo/processinfo.cpp
    src/systeminfo/diskstat.cpp
    src/systeminfo/cpuinfo.cpp
    src/systeminfo/meminfo.cpp
    src/systeminfo/load_avg.cpp
    src/systeminfo/uptime.cpp
    src/systeminfo/filesystems.cpp
    src/systeminfo/sysinfo.cpp
    src/systeminfo/hwmonitor.cpp
)
# Links against Backend to get data
target_link_libraries(telemetry_features PUBLIC telemetry_backend)

# =========================================================
# LIBRARY 3: Runtime (Top Layer)
# Orchestration, Config Parsing, and Main Loops.
# This depends on everything below it.
# =========================================================
add_library(telemetry_runtime STATIC
    src/lua_parser.cpp
    src/parser.cpp
    src/runner.cpp
    src/conky/conky_format.cpp
    src/conky/conky_output.cpp
)
target_link_libraries(telemetry_runtime PUBLIC telemetry_features ${LUA_LIBRARIES})
target_include_directories(telemetry_runtime PUBLIC ${LUA_INCLUDE_DIR})

# =========================================================
# EXECUTABLES
# =========================================================

add_executable(waybard src/waybar/waybar_formatters.cpp src/waybar.cpp)
target_link_libraries(waybard PRIVATE telemetry_runtime)

add_executable(json src/json.cpp)
target_link_libraries(json PRIVATE telemetry_runtime)

add_executable(widgets src/widgets/main.cpp src/widgets/widgets.cpp src/widgets/window.cpp)
target_link_libraries(widgets PRIVATE telemetry_runtime ${GTKMM_LIBRARIES})
target_include_directories(widgets PRIVATE ${GTKMM_INCLUDE_DIRS})

install(TARGETS waybard json widgets RUNTIME DESTINATION bin)
install(TARGETS json COMPONENT json RUNTIME DESTINATION bin)
