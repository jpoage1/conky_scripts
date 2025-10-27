CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 \
  -Isrc/diskstat/include \
  -Isrc/data \
  -Isrc/ssh \
  -Isrc/corestat \
  -Isrc/cpuinfo \
  -Isrc/meminfo \
  -Isrc/networkstats \
  -Isrc/swapinfo \
  -Isrc/uptime

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
SRC_SSH = src/ssh/ssh.cpp
SRC_CORESTAT = src/corestat/corestat.cpp
SRC_CPUINFO = src/cpuinfo/cpuinfo.cpp
SRC_MEMINFO = src/meminfo/meminfo.cpp
SRC_NETWORKSTATS = src/networkstats/networkstats.cpp
SRC_SWAPINFO = src/swapinfo/swapinfo.cpp
SRC_UPTIME = src/uptime/uptime.cpp
SRC_MAIN = src/conkyd.cpp
SRC_WAYBAR = src/waybar.cpp

# Object files
OBJ_DATA = $(OBJ_DIR)/data/data.o
OBJ_DATA_LOCAL = $(OBJ_DIR)/data/data_local.o
OBJ_DATA_SSH = $(OBJ_DIR)/data/data_ssh.o
OBJ_METRICS = $(OBJ_DIR)/data/metrics.o
OBJ_SSH = $(OBJ_DIR)/ssh/ssh.o
OBJ_CORESTAT = $(OBJ_DIR)/corestat/corestat.o
OBJ_CPUINFO = $(OBJ_DIR)/cpuinfo/cpuinfo.o
OBJ_MEMINFO = $(OBJ_DIR)/meminfo/meminfo.o
OBJ_NETWORKSTATS = $(OBJ_DIR)/networkstats/networkstats.o
OBJ_SWAPINFO = $(OBJ_DIR)/swapinfo/swapinfo.o
OBJ_UPTIME = $(OBJ_DIR)/uptime/uptime.o
OBJ_CONKYD = $(OBJ_DIR)/conkyd.o
OBJ_WAYBARD = $(OBJ_DIR)/waybar.o

# Diskstat sources/objects
DISKSTAT_SRC = $(wildcard $(DISKSTAT_SRC_DIR)/*.cpp)
DISKSTAT_OBJ = $(patsubst $(DISKSTAT_SRC_DIR)/%.cpp,$(DISKSTAT_OBJ_DIR)/%.o,$(DISKSTAT_SRC))

# Final binary
CONKYD_BIN = conkyd.bin
WAYBARD_BIN = waybard.bin

.PHONY: all clean debug

all: $(DISKSTAT_LIB) $(BIN)

$(CONKYD_BIN): $(OBJ_DATA) $(OBJ_DATA_LOCAL) $(OBJ_DATA_SSH) $(OBJ_METRICS) $(OBJ_SSH) $(OBJ_CORESTAT) $(OBJ_CPUINFO) $(OBJ_MEMINFO) $(OBJ_NETWORKSTATS) $(OBJ_SWAPINFO) $(OBJ_UPTIME) $(OBJ_CONKYD) $(DISKSTAT_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^  -lssh

$(WAYBARD_BIN): $(OBJ_DATA) $(OBJ_DATA_LOCAL) $(OBJ_DATA_SSH) $(OBJ_METRICS) $(OBJ_SSH) $(OBJ_CORESTAT) $(OBJ_CPUINFO) $(OBJ_MEMINFO) $(OBJ_NETWORKSTATS) $(OBJ_SWAPINFO) $(OBJ_UPTIME) $(OBJ_WAYBARD) $(DISKSTAT_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^  -lssh

# Build static lib
$(DISKSTAT_LIB): $(DISKSTAT_OBJ)
	mkdir -p $(dir $@)
	ar rcs $@ $^

# Build diskstat object files
$(DISKSTAT_OBJ_DIR)/%.o: $(DISKSTAT_SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build other object files
$(OBJ_CORESTAT): src/corestat/corestat.cpp
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

$(OBJ_SSH): src/ssh/ssh.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_CPUINFO): src/cpuinfo/cpuinfo.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_MEMINFO): src/meminfo/meminfo.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_NETWORKSTATS): src/networkstats/networkstats.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_SWAPINFO): src/swapinfo/swapinfo.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_UPTIME): src/uptime/uptime.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_CONKYD): src/conkyd.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_WAYBARD): src/waybar.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(DISKSTAT_OBJ_DIR) $(DISKSTAT_LIB) $(WAYBARD_BIN) $(CONKYD_BIN)

install: $(CONKYD_BIN) $(WAYBARD_BIN)
	mkdir -p $(INSTALL_DIR)
	cp $(CONKYD_BIN) $(INSTALL_DIR)/conkyd
	cp $(WAYBARD_BIN) $(INSTALL_DIR)/warbard
