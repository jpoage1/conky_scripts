cmake_minimum_required(VERSION 3.16)
project(Telemetry VERSION 1.0 LANGUAGES CXX)

# --- Build Options ---
option(ENABLE_QT_SOCKETS "Enable Qt sockets" ON)
option(ENABLE_QT "Enable Qt output mode" ON)
option(ENABLE_JSON "Enable JSON output mode" ON)
option(ENABLE_CONKY "Enable Conky output mode" OFF)
option(ENABLE_GTK "Enable GTK output mode" OFF)
option(ENABLE_GTK_SOCKETS "Enable GTK sockets" OFF)
option(ENABLE_TESTS "Build unit tests" ON)

# --- Configuration ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# --- Dependencies ---
find_package(PkgConfig REQUIRED)
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
add_library(telemetry_core STATIC
    # Data & Streams
    src/data/data.cpp
    src/data/data_local.cpp
    src/data/data_ssh.cpp
    src/data/metrics.cpp
    src/data/polling.cpp
    src/data/ssh.cpp
    
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
    src/systeminfo/system_stability.cpp
    src/systeminfo/frag_stats.cpp
    
    # Logging
    src/logging/io.cpp
    src/logging/rotate_file_sync.cpp
    src/logging/teebuf.cpp
    src/logging/log.cpp

    # Core
    src/core/controller.cpp
    src/core/configuration_builder.cpp
    src/core/parsed_config.cpp
    src/core/telemetry.cpp
    src/core/lua_parser.cpp
    src/core/cli_parser.cpp
    src/core/runner.cpp
    src/core/runner_context.cpp
)


# Link external deps to the core library
target_link_libraries(telemetry_core PUBLIC
    ${LIBSSH_LIBRARIES}
    stdc++fs ${LUA_LIBRARIES}
    spdlog::spdlog
)
target_include_directories(telemetry_core PUBLIC ${LIBSSH_INCLUDE_DIRS} ${LUA_INCLUDE_DIR})
add_executable(telemetry src/main.cpp)
target_link_libraries(telemetry PRIVATE telemetry_core)

if(ENABLE_QT OR ENABLE_GTK)
    set(ENABLE_JSON ON CACHE BOOL "Forced ON by Qt/GTK dependency" FORCE)
    message(STATUS "Note: ENABLE_JSON forced ON due to ENABLE_QT or ENABLE_GTK")
endif()

if(ENABLE_TESTS)
    enable_testing()
    add_executable(telemetry_tests
        tests/unit_json_definitions.cpp
        tests/unit_corestat.cpp
        tests/unit_json_order.cpp
        tests/unit_stability.cpp
        tests/unit_frag_stats.cpp
        tests/unit_process_io.cpp
        tests/main.cpp
    )
    target_link_libraries(telemetry_tests PRIVATE
        telemetry_core 
        GTest::gtest
    )
    add_test(NAME AllUnits COMMAND telemetry_tests)
endif()

if(ENABLE_CONKY)
    add_library(telemetry_conky STATIC
        src/conky/conky_format.cpp
        src/conky/conky_output.cpp
        src/conky/conky_pipeline.cpp
        src/conky/conky_main.cpp
        src/main.cpp
    )
    target_link_libraries(telemetry_conky PUBLIC telemetry_core)
    target_compile_definitions(telemetry_conky PUBLIC OUTPUT_MODE_CONKY)
    target_link_libraries(telemetry PRIVATE telemetry_conky)
    target_link_libraries(telemetry_tests PRIVATE telemetry_conky)
endif()

if(ENABLE_JSON)
    add_library(telemetry_json STATIC
        src/json/json_pipeline.cpp
        src/json/json_serializer.cpp
        src/json/json_definitions.cpp
        src/json/json_main.cpp
    )
    target_link_libraries(telemetry_json PUBLIC telemetry_core)
    target_compile_definitions(telemetry_json PUBLIC OUTPUT_MODE_JSON)
    target_link_libraries(telemetry_tests PRIVATE telemetry_json)
endif()

if(ENABLE_QT)
    find_package(Qt6 REQUIRED COMPONENTS Core Quick Widgets)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    Qt6_add_resources(QT_RESOURCES resources.qrc)
    add_library(telemetry_qt STATIC
        src/qt_widgets/qt_pipeline.cpp
        src/qt_widgets/qt_main.cpp
        includes/window_settings.hpp
        includes/system_metrics_proxy.hpp
        includes/system_metrics_qt_proxy.hpp
        ${QT_RESOURCES}
    )
    target_link_libraries(telemetry_qt PUBLIC
        telemetry_core
        telemetry_json
        Qt6::Core
        Qt6::Quick 
        Qt6::Widgets
    )
    target_compile_definitions(telemetry_qt PUBLIC OUTPUT_MODE_QT)
    target_link_libraries(telemetry PRIVATE telemetry_qt)
    target_link_libraries(telemetry_tests PRIVATE telemetry_qt)
endif()

if(ENABLE_GTK)
    pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
    target_compile_definitions(telemetry_gtk PUBLIC OUTPUT_MODE_GTK)
endif()

if (ENABLE_QT_SOCKETS)
    find_package(Qt6 REQUIRED COMPONENTS WebSockets)
    add_library(telemetry_qt_sockets)
    target_sources(telemetry_core PRIVATE )
    target_link_libraries(telemetry PUBLIC Qt6::WebSockets)
    target_link_libraries(telemetry_tests PUBLIC Qt6::WebSockets)
    target_compile_definitions(telemetry_qt_sockets PUBLIC OUTPUT_MODE_QT_SOCKETS)
endif()

if (ENABLE_GTK_SOCKETS)
    add_library(telemetry_gtk_sockets)
    target_sources(telemetry_core PRIVATE src/network/soup_socket_impl.cpp)
    target_link_libraries(telemetry PUBLIC libsoup-3.0)
    target_link_libraries(telemetry_tests PUBLIC libsoup-3.0)
    target_compile_definitions(telemetry_gtk_sockets PUBLIC OUTPUT_MODE_GTK_SOCKETS)
endif()
