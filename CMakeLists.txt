cmake_minimum_required(VERSION 3.16)
project(telemetry VERSION 1.0 LANGUAGES CXX)

# --- 1. Global Toolchain & Optimization ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- RPATH Configuration ---
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# --- Legacy Include Pathing ---
include_directories(
    includes
    src/systeminfo
    src/cli
    src/processinfo
    src/data
    src/widgets
)

# Enable Qt Auto-processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Optimization & Hardening
add_compile_options(-Wall -Wextra -O3 -march=native)
add_link_options(-fsanitize=address)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Link with mold for NixOS performance
    add_link_options("-fuse-ld=mold")
endif()

# Fast Rebuilds
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# --- 2. Dependencies ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
pkg_check_modules(LIBSSH REQUIRED libssh)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Widgets)
find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
find_package(Lua REQUIRED)
find_package(Threads REQUIRED)

# --- 3. Interface Libraries (Shared Settings) ---
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra)

add_library(project_pch INTERFACE)
target_precompile_headers(project_pch INTERFACE
    <string> <vector> <map> <functional>
    <spdlog/spdlog.h>
    <QtCore/QObject>
)

# --- 4. Layered Static Libraries ---

# LAYER 1: Low-level Data Acquisition
add_library(telemetry_backend STATIC
    src/data/data.cpp
    src/data/data_local.cpp
    src/data/data_ssh.cpp
    src/ssh.cpp
)

target_compile_definitions(telemetry_backend PUBLIC SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
target_link_libraries(telemetry_backend PUBLIC 
    PkgConfig::LIBSSH 
    spdlog::spdlog 
    Threads::Threads
)
target_include_directories(telemetry_backend PUBLIC includes ${LIBSSH_INCLUDE_DIRS})

target_include_directories(telemetry_backend PUBLIC includes)

# LAYER 2: Feature & Metric Logic
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
    src/systeminfo/batteryinfo.cpp
)
target_link_libraries(telemetry_features PUBLIC telemetry_backend project_pch)

# LAYER 3: Runtime & Orchestration
add_library(telemetry_runtime STATIC
    src/lua_parser.cpp
    src/cli_parser.cpp
    src/parsed_config.cpp
    src/runner.cpp
    src/conky/conky_format.cpp
    src/conky/conky_output.cpp
    src/conky/conky_pipeline.cpp
    src/json/json_pipeline.cpp
    src/json/json_serializer.cpp
    src/json/json_definitions.cpp
    src/io.cpp
    src/rotate_file_sync.cpp
    src/teebuf.cpp
    src/log.cpp
    src/controller.cpp
    src/configuration_builder.cpp
)
target_link_libraries(telemetry_runtime PUBLIC 
    telemetry_features 
    ${LUA_LIBRARIES} 
    Qt6::Core
)
target_include_directories(telemetry_runtime PUBLIC ${LUA_INCLUDE_DIR})

# --- 5. Executables ---

# Main Telemetry (Qt/QML based)
add_executable(telemetry_bin 
    src/qt_main.cpp
    # resources.qrc # Uncomment if you have assets
)
set_target_properties(telemetry_bin PROPERTIES OUTPUT_NAME "telemetry")
target_link_libraries(telemetry_bin PRIVATE 
    telemetry_runtime 
    Qt6::Quick 
    Qt6::Widgets
)

# Standalone CLI Telemetry
add_executable(telemetry src/telemetry.cpp)
target_link_libraries(telemetry PRIVATE telemetry_runtime)

# Legacy GTK Widgets executable
add_executable(telemetry_widgets 
    src/widgets/main.cpp 
    src/widgets/widgets.cpp 
    src/widgets/window.cpp
)

target_link_libraries(telemetry_widgets PRIVATE 
    telemetry_runtime 
    PkgConfig::GTKMM
)

# Alternative Widgets naming (requested section)
add_executable(widgets src/widgets/main.cpp src/widgets/widgets.cpp src/widgets/window.cpp)
target_link_libraries(widgets PRIVATE telemetry_runtime ${GTKMM_LIBRARIES})
target_include_directories(widgets PRIVATE ${GTKMM_INCLUDE_DIRS})

# Waybar Tool Tip
# add_executable(waybard src/waybar/waybar_formatters.cpp src/waybar.cpp)
# target_link_libraries(waybard PRIVATE telemetry_runtime)

# Conky Polling
# add_executable(conkyd src/conky_main.cpp)
# target_link_libraries(waybard PRIVATE telemetry_runtime)

# --- 6. Testing ---
enable_testing()
add_executable(unit_tests tests/main.cpp)
target_link_libraries(unit_tests PRIVATE 
    telemetry_runtime 
    GTest::gtest_main
)
add_test(NAME AllUnits COMMAND unit_tests)

# --- 7. Installation ---
install(TARGETS telemetry_bin telemetry_widgets RUNTIME DESTINATION bin)
# install(TARGETS telemetry COMPONENT telemetry RUNTIME DESTINATION bin)
