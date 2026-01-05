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

find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)

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
    src/data/polling.cpp
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
    src/systeminfo/batteryinfo.cpp
    
    # Runners & Parsers
    src/
    src/lua_parser.cpp
    src/cli_parser.cpp
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
    src/parsed_config.cpp
)

add_library(telemetry_unit_tests STATIC
    tests/unit_corestat.cpp
    tests/unit_json_order.cpp
)

# Link external deps to the core library
target_link_libraries(telemetry_core PUBLIC
    ${LIBSSH_LIBRARIES}
    stdc++fs ${LUA_LIBRARIES}
    spdlog::spdlog
)
target_include_directories(telemetry_core PUBLIC ${LIBSSH_INCLUDE_DIRS} ${LUA_INCLUDE_DIR})


# --- Executable: telemetry ---
add_executable(telemetry src/telemetry.cpp)
target_link_libraries(telemetry PRIVATE telemetry_core)

enable_testing()
add_executable(unit_tests tests/main.cpp)
target_link_libraries(unit_tests PRIVATE
    telemetry_core 
    telemetry_unit_tests
    GTest::gtest
)

# --- Install ---
install(TARGETS telemetry COMPONENT telemetry RUNTIME DESTINATION bin)

enable_testing()

add_test(NAME AllUnits COMMAND unit_tests)
