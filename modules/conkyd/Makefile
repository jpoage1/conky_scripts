CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 \
  -Isrc/diskstat/include \
  -Isrc/systeminfo \
  -Isrc/processinfo \
  -Isrc/data \
  -Isrc/widgets \
  -Iincludes

GTK_CFLAGS = $(shell pkg-config --cflags gtkmm-4.0)
GTK_LIBS = $(shell pkg-config --libs gtkmm-4.0)
WIDGETS_CXXFLAGS = -std=c++17 -Wall -Wextra -O2 $(GTK_CFLAGS)

# Directories
INSTALL_DIR = $(HOME)/.config/conky/bin
DISKSTAT_DIR = src/diskstat
DISKSTAT_SRC_DIR = $(DISKSTAT_DIR)/src
SYSTEMINFO_SRC_DIR = src/systeminfo


OBJ_DIR = obj
DISKSTAT_OBJ_DIR = $(DISKSTAT_DIR)/obj
SYSTEMINFO_OBJ_DIR = $(OBJ_DIR)/systeminfo

# LIBS
DISKSTAT_LIB = $(DISKSTAT_DIR)/libdiskstat.a
SYSTEMINFO_LIB = $(OBJ_DIR)/systeminfo.a

# Source files
SRC_DATA = src/data/data.cpp
SRC_DATA_LOCAL = src/data/data_local.cpp
SRC_DATA_SSH = src/data/data_ssh.cpp
SRC_SSH = src/ssh.cpp
SRC_CORESTAT = src/systeminfo/corestat.cpp
SRC_CPUINFO = src/systeminfo/cpuinfo.cpp
SRC_MEMINFO = src/systeminfo/meminfo.cpp
SRC_PROCESSINFO = src/systeminfo/networkstats.cpp
SRC_MEM_PROCESSES = src/systeminfo/mem_processes.cpp
SRC_CPU_PROCESSES = src/systeminfo/cpu_processes.cpp
SRC_LOAD_AVG = src/systeminfo/load_avg.cpp
SRC_UPTIME = src/systeminfo/uptime.cpp
SRC_DISKSTAT = src/systeminfo/diskstat.cpp
SRC_SYSINFO = src/systeminfo/sysinfo.cpp
SRC_HWMONITOR = src/systeminfo/hwmonitor.cpp
SRC_WAYBAR_TYPES = src/waybar_types.cpp
SRC_CLI_PARSER = src/parser.cpp
SRC_CONKY_FORMAT = src/diskstat/src/conky_format.cpp
SRC_WAYBAR_FORMATTER = src/waybar/waybar_formatters.cpp
SRC_WAYBAR_MAIN = src/waybar.cpp
SRC_JSON_MAIN = src/json.cpp
SRC_WAYBAR = src/waybar.cpp
SRC_WIDGETS = src/widgets/main.cpp

# Object files
OBJ_DATA = $(OBJ_DIR)/data/data.o
OBJ_DATA_LOCAL = $(OBJ_DIR)/data/data_local.o
OBJ_DATA_SSH = $(OBJ_DIR)/data/data_ssh.o
OBJ_METRICS = $(OBJ_DIR)/data/metrics.o
OBJ_SSH = $(OBJ_DIR)/ssh.o
OBJ_CORESTAT = $(OBJ_DIR)/systeminfo/corestat.o
OBJ_CPUINFO = $(OBJ_DIR)/systeminfo/cpuinfo.o
OBJ_MEMINFO = $(OBJ_DIR)/systeminfo/meminfo.o
OBJ_NETWORKSTATS = $(OBJ_DIR)/systeminfo/networkstats.o
OBJ_MEM_PROCESSES = $(OBJ_DIR)/systeminfo/mem_processes.o
OBJ_CPU_PROCESSES = $(OBJ_DIR)/systeminfo/cpu_processes.o
OBJ_LOAD_AVG = $(OBJ_DIR)/systeminfo/load_avg.o
OBJ_UPTIME = $(OBJ_DIR)/systeminfo/uptime.o
OBJ_DISKSTAT = $(OBJ_DIR)/systeminfo/diskstat.o
OBJ_SYSINFO = $(OBJ_DIR)/systeminfo/sysinfo.o
OBJ_HWMONITOR = $(OBJ_DIR)/systeminfo/hwmonitor.o
OBJ_CLI_PARSER = $(OBJ_DIR)/cli_parser.o
OBJ_CONKY_FORMAT = $(OBJ_DIR)/conky_format.o
OBJ_WAYBAR_FORMATTER = $(OBJ_DIR)/waybar_formatter.o
OBJ_WAYBAR_MAIN = $(OBJ_DIR)/waybar.O
OBJ_WAYBARD = $(OBJ_DIR)/waybar.o
OBJ_JSON_MAIN = $(OBJ_DIR)/json.o
OBJ_WIDGETS_WIDGETS = $(OBJ_DIR)/widgets_widgets.o
OBJ_WIDGETS_MAIN = $(OBJ_DIR)/widgets_main.o
OBJ_WIDGETS_WINDOW = $(OBJ_DIR)/widgets_window.o

# Object file collections
SYSTEMINFO_OBJ = \
    $(OBJ_CORESTAT) $(OBJ_CPUINFO) $(OBJ_MEMINFO) $(OBJ_NETWORKSTATS) \
    $(OBJ_MEM_PROCESSES) $(OBJ_CPU_PROCESSES) $(OBJ_LOAD_AVG) \
   $(OBJ_UPTIME) $(OBJ_DISKSTAT) $(OBJ_SYSINFO) $(OBJ_HWMONITOR)

# Diskstat sources/objects
DISKSTAT_SRC = $(wildcard $(DISKSTAT_SRC_DIR)/*.cpp)
DISKSTAT_OBJ = $(patsubst $(DISKSTAT_SRC_DIR)/%.cpp,$(DISKSTAT_OBJ_DIR)/%.o,$(DISKSTAT_SRC))


# Final binary
WAYBARD_BIN = waybard.bin
JSON_BIN = json.bin
WIDGETS_BIN = widgets.bin

.PHONY: all clean debug waybard json widgets

all: $(DISKSTAT_LIB) $(SYSTEMINFO_LIB) $(WAYBARD_BIN) $(JSON_BIN) $(WIDGETS_BIN)

$(WAYBARD_BIN): $(OBJ_DATA) $(OBJ_DATA_LOCAL) $(OBJ_DATA_SSH) $(OBJ_METRICS) $(OBJ_SSH) $(OBJ_DISKSTAT) $(OBJ_CLI_PARSER) $(OBJ_WAYBAR_FORMATTER) $(OBJ_WAYBAR_MAIN) $(DISKSTAT_LIB) $(SYSTEMINFO_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^  -lssh -lstdc++fs

$(JSON_BIN): $(OBJ_DATA) $(OBJ_DATA_LOCAL) $(OBJ_DATA_SSH) $(OBJ_METRICS) $(OBJ_SSH) $(OBJ_CLI_PARSER) $(OBJ_DISKSTAT) $(OBJ_WAYBAR_FORMATTER) $(OBJ_CONKY_FORMAT) $(OBJ_JSON_MAIN) $(DISKSTAT_LIB) $(SYSTEMINFO_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^  -lssh -lstdc++fs

$(WIDGETS_BIN): $(OBJ_DATA) $(OBJ_DATA_LOCAL) $(OBJ_DATA_SSH) $(OBJ_METRICS) $(OBJ_SSH) $(OBJ_CLI_PARSER) $(OBJ_DISKSTAT) $(OBJ_WAYBAR_FORMATTER) $(OBJ_WIDGETS_WIDGETS) $(OBJ_WIDGETS_WINDOW) $(OBJ_WIDGETS_MAIN) $(DISKSTAT_LIB) $(SYSTEMINFO_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^  -lssh -lstdc++fs  $(GTK_LIBS)

# Build static lib
$(DISKSTAT_LIB): $(DISKSTAT_OBJ)
	mkdir -p $(dir $@)
	ar rcs $@ $^

# Build diskstat object files
$(DISKSTAT_OBJ_DIR)/%.o: $(DISKSTAT_SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build static lib
$(SYSTEMINFO_LIB): $(SYSTEMINFO_OBJ)
	mkdir -p $(dir $@)
	ar rcs $@ $^

# Build other object files
$(OBJ_CORESTAT): $(SRC_CORESTAT)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Build other object files
$(OBJ_DATA): $(SRC_DATA)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build other object files
$(OBJ_DATA_LOCAL): $(SRC_DATA_LOCAL)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build other object files
$(OBJ_DATA_SSH): $(SRC_DATA_SSH)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_METRICS): src/data/metrics.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_SSH): $(SRC_SSH)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_CPUINFO): $(SRC_CPUINFO)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_MEMINFO): $(SRC_MEMINFO)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_NETWORKSTATS): $(SRC_PROCESSINFO)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_MEM_PROCESSES): $(SRC_MEM_PROCESSES)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_CPU_PROCESSES): $(SRC_CPU_PROCESSES)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_LOAD_AVG): $(SRC_LOAD_AVG)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_UPTIME): $(SRC_UPTIME)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DISKSTAT): $(SRC_DISKSTAT)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_SYSINFO): $(SRC_SYSINFO)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_HWMONITOR): $(SRC_HWMONITOR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_CLI_PARSER): $(SRC_CLI_PARSER)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_CONKY_FORMAT): $(SRC_CONKY_FORMAT)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_WAYBAR_FORMATTER): $(SRC_WAYBAR_FORMATTER)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_WAYBAR_MAIN): $(SRC_WAYBAR_MAIN)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_JSON_MAIN): $(SRC_JSON_MAIN)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_WAYBARD): $(SRC_WAYBAR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_WIDGETS_MAIN): $(SRC_WIDGETS)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -c $< -o $@

$(OBJ_WIDGETS_WIDGETS): src/widgets/widgets.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -c $< -o $@

$(OBJ_WIDGETS_WINDOW): src/widgets/window.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(DISKSTAT_OBJ_DIR) $(DISKSTAT_LIB) $(SYSTEMINFO_LIB) $(WAYBARD_BIN) $(WAYBARD2_BIN) $(WIDGETS_BIN)

install: $(WAYBARD_BIN) $(WAYBARD2_BIN) $(WIDGETS_BIN)
	mkdir -p $(INSTALL_DIR)
	cp $(WAYBARD_BIN) $(INSTALL_DIR)/warbard
	cp $(WAYBARD2_BIN) $(INSTALL_DIR)/warbard2
	cp $(JSON_BIN) $(INSTALL_DIR)/json
	cp $(WIDGETS_BIN) $(INSTALL_DIR)/widgets


waybard: $(WAYBARD_BIN)
	mkdir -p $(INSTALL_DIR)
	cp $(WAYBARD_BIN) $(INSTALL_DIR)/warbard

json: $(JSON_BIN)
	mkdir -p $(INSTALL_DIR)
	cp $(JSON_BIN) $(INSTALL_DIR)/json

widgets: $(WIDGETS_BIN)
	mkdir -p $(INSTALL_DIR)
	cp $(WIDGETS_BIN) $(INSTALL_DIR)/widgets
