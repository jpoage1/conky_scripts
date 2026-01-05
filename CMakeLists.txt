cmake_minimum_required(VERSION 3.16)
project(telemetry VERSION 1.0 LANGUAGES CXX)

# --- 1. Global Toolchain ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Qt Auto-processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Optimization & Tooling
add_compile_options(-Wall -Wextra -O3)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_link_options("-fuse-ld=mold")
endif()

# --- 2. Dependencies ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED IMPORTED_TARGET libssh)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Widgets)
find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
find_package(Lua REQUIRED)
find_package(Threads REQUIRED)

# --- 3. Shared Interface Targets ---
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
target_include_directories(telemetry_backend PUBLIC includes)
target_link_libraries(telemetry_backend PUBLIC
    PkgConfig::LIBSSH 
    spdlog::spdlog
    Threads::Threads
)

# LAYER 2: Feature & Metric Logic
add_library(telemetry_features STATIC
    src/data/metrics.cpp 
    src/data/polling.cpp
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

# Main Telemetry
add_executable(telemetry_bin src/telemetry.cpp)
set_target_properties(telemetry_bin PROPERTIES OUTPUT_NAME "telemetry")

# Linking the main binary to all top-level Qt targets ensures 
# Nix identifies the needed RPATH entries.
target_link_libraries(telemetry_bin PRIVATE 
    telemetry_runtime 
    Qt6::Quick 
    Qt6::Widgets
)

# --- 6. Testing ---
enable_testing()
add_executable(unit_tests tests/main.cpp)
target_link_libraries(unit_tests PRIVATE 
    telemetry_runtime 
    GTest::gtest_main
)
add_test(NAME AllUnits COMMAND unit_tests)

# --- 7. Installation ---
install(TARGETS telemetry_bin RUNTIME DESTINATION bin)
