CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 \
  -Isrc/diskstat/include \
  -Isrc/corestat \
  -Isrc/cpuinfo \
  -Isrc/meminfo \
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
SRC_CORESTAT = src/corestat/corestat.cpp
SRC_CPUINFO = src/cpuinfo/cpuinfo.cpp
SRC_MEMINFO = src/meminfo/meminfo.cpp
SRC_SWAPINFO = src/swapinfo/swapinfo.cpp
SRC_UPTIME = src/uptime/uptime.cpp
SRC_MAIN = src/main.cpp

# Object files
OBJ_CORESTAT = $(OBJ_DIR)/corestat/corestat.o
OBJ_CPUINFO = $(OBJ_DIR)/cpuinfo/cpuinfo.o
OBJ_MEMINFO = $(OBJ_DIR)/meminfo/meminfo.o
OBJ_SWAPINFO = $(OBJ_DIR)/swapinfo/swapinfo.o
OBJ_UPTIME = $(OBJ_DIR)/uptime/uptime.o
OBJ_MAIN = $(OBJ_DIR)/main.o

# Diskstat sources/objects
DISKSTAT_SRC = $(wildcard $(DISKSTAT_SRC_DIR)/*.cpp)
DISKSTAT_OBJ = $(patsubst $(DISKSTAT_SRC_DIR)/%.cpp,$(DISKSTAT_OBJ_DIR)/%.o,$(DISKSTAT_SRC))

# Final binary
BIN = conkyd.bin

.PHONY: all clean

all: $(DISKSTAT_LIB) $(BIN)

$(BIN): $(OBJ_CORESTAT) $(OBJ_CPUINFO) $(OBJ_MEMINFO) $(OBJ_SWAPINFO) $(OBJ_UPTIME) $(OBJ_MAIN) $(DISKSTAT_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^

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

$(OBJ_CPUINFO): src/cpuinfo/cpuinfo.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_MEMINFO): src/meminfo/meminfo.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_SWAPINFO): src/swapinfo/swapinfo.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_UPTIME): src/uptime/uptime.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_MAIN): src/main.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(DISKSTAT_OBJ_DIR) $(DISKSTAT_LIB) $(BIN)

install: $(BIN)
	mkdir -p $(INSTALL_DIR)
	cp $(BIN) $(INSTALL_DIR)/conkyd
