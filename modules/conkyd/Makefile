CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 \
  -Isrc/diskstat/include \
  -Isrc/corestat \
  -Isrc/cpuinfo \
  -Isrc/meminfo \
  -Isrc/swapinfo \
  -Isrc/uptime
LDFLAGS =

# Diskstat static library build
DISKSTAT_DIR = conkyd/src/diskstat
DISKSTAT_SRC = $(wildcard $(DISKSTAT_DIR)/src/*.cpp)
DISKSTAT_OBJ = $(patsubst $(DISKSTAT_DIR)/src/%.cpp,$(DISKSTAT_DIR)/obj/%.o,$(DISKSTAT_SRC))
DISKSTAT_LIB = $(DISKSTAT_DIR)/libdiskstat.a

# Other modules with subdirs under src/
SRC_CORESTAT = src/corestat/corestat.cpp
SRC_CPUINFO = src/cpuinfo/cpuinfo.cpp
SRC_MEMINFO = src/meminfo/meminfo.cpp
SRC_SWAPINFO = src/swapinfo/swapinfo.cpp
SRC_UPTIME = src/uptime/uptime.cpp
SRC_CONKYD = src/main.cpp


SRC_OTHER = $(SRC_CORESTAT) $(SRC_CPUINFO) $(SRC_MEMINFO) $(SRC_SWAPINFO) $(SRC_UPTIME) $(SRC_CONKYD)
OBJ_DIR = obj
OBJ_OTHER = $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRC_OTHER))

BIN = conkyd.bin

.PHONY: all clean

all: $(DISKSTAT_LIB) $(BIN)

# Link main executable with diskstat static lib
$(BIN): $(OBJ_OTHER) $(DISKSTAT_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Build diskstat static library
$(DISKSTAT_LIB): $(DISKSTAT_OBJ)
	mkdir -p $(dir $@)
	ar rcs $@ $^

# Compile diskstat objects
$(DISKSTAT_DIR)/obj/%.o: $(DISKSTAT_DIR)/src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile other object files preserving directory structure inside obj/
$(OBJ_DIR)/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(DISKSTAT_DIR)/obj/*.o $(DISKSTAT_LIB)
	rm -rf $(OBJ_DIR) $(BIN)
