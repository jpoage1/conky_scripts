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
OBJ_DIR = obj
INSTALL_DIR = $(HOME)/.config/conky/bin
DISKSTAT_DIR = src/diskstat
DISKSTAT_SRC_DIR = $(DISKSTAT_DIR)/src
DISKSTAT_OBJ_DIR = $(DISKSTAT_DIR)/obj
DISKSTAT_LIB = $(DISKSTAT_DIR)/libdiskstat.a

# Source files
SRC_DATA = src/data/data.cpp
SRC_DATA_LOCAL = src/data/data_local.cpp
SRC_DATA_SSH = src/data/data_ssh.cpp
SRC_SSH = src/ssh.cpp
SRC_CORESTAT = src/systeminfo/corestat.cpp
SRC_CPUINFO = src/systeminfo/cpuinfo.cpp
SRC_MEMINFO = src/systeminfo/meminfo.cpp
SRC_PROCESSINFO = src/systeminfo/networkstats.cpp
SRC_NETWORKSTATS = src/processinfo/processinfo.cpp
SRC_SWAPINFO = src/systeminfo/swapinfo.cpp
SRC_UPTIME = src/systeminfo/uptime.cpp
SRC_DISKSTAT = src/systeminfo/diskstat.cpp
SRC_MAIN = src/conkyd.cpp
SRC_WAYBAR_TYPES = src/waybar_types.cpp
SRC_CLI_PARSER = src/parser.cpp
SRC_WAYBAR_FORMATTER = src/waybar_formatter.cpp
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
OBJ_PROCESSINFO = $(OBJ_DIR)/processinfo/processinfo.o
OBJ_SWAPINFO = $(OBJ_DIR)/systeminfo/swapinfo.o
OBJ_UPTIME = $(OBJ_DIR)/systeminfo/uptime.o
OBJ_DISKSTAT = $(OBJ_DIR)/systeminfo/diskstat.o
OBJ_CLI_PARSER = $(OBJ_DIR)/cli_parser.o
OBJ_WAYBAR_FORMATTER = $(OBJ_DIR)/waybar_formatter.o
OBJ_WAYBAR_MAIN = $(OBJ_DIR)/waybar.O
OBJ_CONKYD = $(OBJ_DIR)/conkyd.o
OBJ_WAYBARD = $(OBJ_DIR)/waybar.o
OBJ_JSON_MAIN = $(OBJ_DIR)/json.o
OBJ_WIDGETS_WIDGETS = $(OBJ_DIR)/widgets_widgets.o
OBJ_WIDGETS_MAIN = $(OBJ_DIR)/widgets_main.o
OBJ_WIDGETS_WINDOW = $(OBJ_DIR)/widgets_window.o

# Diskstat sources/objects
DISKSTAT_SRC = $(wildcard $(DISKSTAT_SRC_DIR)/*.cpp)
DISKSTAT_OBJ = $(patsubst $(DISKSTAT_SRC_DIR)/%.cpp,$(DISKSTAT_OBJ_DIR)/%.o,$(DISKSTAT_SRC))

# Final binary
CONKYD_BIN = conkyd.bin
WAYBARD_BIN = waybard.bin
WAYBARD2_BIN = waybard2.bin
JSON_BIN = json.bin
WIDGETS_BIN = widgets.bin

.PHONY: all clean debug waybard json widgets

all: $(DISKSTAT_LIB) $(CONKY_BIN) $(WAYBARD_BIN) (JSON_BIN) $(WIDGETS_BIN)

$(CONKYD_BIN): $(OBJ_DATA) $(OBJ_DATA_LOCAL) $(OBJ_DATA_SSH) $(OBJ_METRICS) $(OBJ_SSH) $(OBJ_CORESTAT) $(OBJ_CPUINFO) $(OBJ_MEMINFO) $(OBJ_NETWORKSTATS) $(OBJ_PROCESSINFO) $(OBJ_SWAPINFO) $(OBJ_UPTIME) $(OBJ_DISKSTAT) $(OBJ_CONKYD) $(DISKSTAT_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^  -lssh -lstdc++fs

$(WAYBARD_BIN): $(OBJ_DATA) $(OBJ_DATA_LOCAL) $(OBJ_DATA_SSH) $(OBJ_METRICS) $(OBJ_SSH) $(OBJ_CORESTAT) $(OBJ_CPUINFO) $(OBJ_MEMINFO) $(OBJ_NETWORKSTATS) $(OBJ_PROCESSINFO) $(OBJ_SWAPINFO) $(OBJ_UPTIME) $(OBJ_DISKSTAT) $(OBJ_CLI_PARSER) $(OBJ_WAYBAR_FORMATTER) $(OBJ_WAYBAR_MAIN) $(DISKSTAT_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^  -lssh -lstdc++fs

$(JSON_BIN): $(OBJ_DATA) $(OBJ_DATA_LOCAL) $(OBJ_DATA_SSH) $(OBJ_METRICS) $(OBJ_SSH) $(OBJ_CORESTAT) $(OBJ_CPUINFO) $(OBJ_MEMINFO) $(OBJ_NETWORKSTATS) $(OBJ_PROCESSINFO) $(OBJ_SWAPINFO) $(OBJ_UPTIME) $(OBJ_CLI_PARSER) $(OBJ_DISKSTAT) $(OBJ_WAYBAR_FORMATTER) $(OBJ_JSON_MAIN) $(DISKSTAT_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^  -lssh -lstdc++fs

$(WIDGETS_BIN): $(OBJ_DATA) $(OBJ_DATA_LOCAL) $(OBJ_DATA_SSH) $(OBJ_METRICS) $(OBJ_SSH) $(OBJ_CORESTAT) $(OBJ_CPUINFO) $(OBJ_MEMINFO) $(OBJ_NETWORKSTATS) $(OBJ_PROCESSINFO) $(OBJ_SWAPINFO) $(OBJ_UPTIME) $(OBJ_CLI_PARSER) $(OBJ_DISKSTAT) $(OBJ_WAYBAR_FORMATTER) $(OBJ_WIDGETS_WIDGETS) $(OBJ_WIDGETS_WINDOW) $(OBJ_WIDGETS_MAIN) $(DISKSTAT_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^  -lssh -lstdc++fs  $(GTK_LIBS)

# Build static lib
$(DISKSTAT_LIB): $(DISKSTAT_OBJ)
	mkdir -p $(dir $@)
	ar rcs $@ $^

# Build diskstat object files
$(DISKSTAT_OBJ_DIR)/%.o: $(DISKSTAT_SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build other object files
$(OBJ_CORESTAT): src/systeminfo/corestat.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Build other object files
$(OBJ_DATA): src/data/data.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build other object files
$(OBJ_DATA_LOCAL): src/data/data_local.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build other object files
$(OBJ_DATA_SSH): src/data/data_ssh.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_METRICS): src/data/metrics.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_SSH): src/ssh.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_CPUINFO): src/systeminfo/cpuinfo.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_MEMINFO): src/systeminfo/meminfo.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_NETWORKSTATS): src/systeminfo/networkstats.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_PROCESSINFO): src/processinfo/processinfo.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_SWAPINFO): src/systeminfo/swapinfo.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_UPTIME): src/systeminfo/uptime.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DISKSTAT): src/systeminfo/diskstat.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(OBJ_CLI_PARSER): src/parser.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_WAYBAR_FORMATTER): src/waybar/waybar_formatter.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_WAYBAR_MAIN): src/waybar.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_JSON_MAIN): src/json.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_CONKYD): src/conkyd.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_WAYBARD): src/waybar.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_WIDGETS_MAIN): src/widgets/main.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -c $< -o $@

$(OBJ_WIDGETS_WIDGETS): src/widgets/widgets.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -c $< -o $@

$(OBJ_WIDGETS_WINDOW): src/widgets/window.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(DISKSTAT_OBJ_DIR) $(DISKSTAT_LIB) $(WAYBARD_BIN) $(WAYBARD2_BIN) $(CONKYD_BIN) $(WIDGETS_BIN)

install: $(CONKYD_BIN) $(WAYBARD_BIN) $(WAYBARD2_BIN) $(WIDGETS_BIN)
	mkdir -p $(INSTALL_DIR)
	cp $(CONKYD_BIN) $(INSTALL_DIR)/conkyd
	cp $(WAYBARD_BIN) $(INSTALL_DIR)/warbard
	cp $(WAYBARD2_BIN) $(INSTALL_DIR)/warbard2
	cp $(JSON_BIN) $(INSTALL_DIR)/json
	cp $(WIDGETS_BIN) $(INSTALL_DIR)/widgets


waybard: $(WAYBARD_BIN)
	mkdir -p $(INSTALL_DIR)
	cp $(WAYBARD_BIN) $(INSTALL_DIR)/warbard

waybard2: $(WAYBARD2_BIN)
	mkdir -p $(INSTALL_DIR)
	cp $(WAYBARD2_BIN) $(INSTALL_DIR)/warbard2

json: $(JSON_BIN)
	mkdir -p $(INSTALL_DIR)
	cp $(JSON_BIN) $(INSTALL_DIR)/json

widgets: $(WIDGETS_BIN)
	mkdir -p $(INSTALL_DIR)
	cp $(WIDGETS_BIN) $(INSTALL_DIR)/widgets
