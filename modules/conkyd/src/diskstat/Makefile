CXX = g++

BASEFLAGS = -std=c++17 -Wall -Wextra -Iinclude
OPTFLAGS  = -O2
DBGFLAGS  = -g -O0

SRC_DIR = src
OBJ_DIR = obj
BIN = diskstat
INSTALL_DIR = $(HOME)/.config/conky/bin

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))

.PHONY: all debug clean install

all: CXXFLAGS = $(BASEFLAGS) $(OPTFLAGS)
all: $(BIN)

debug: CXXFLAGS = $(BASEFLAGS) $(DBGFLAGS)
debug: $(BIN)

$(BIN): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN)

install: $(BIN)
	@mkdir -p $(INSTALL_DIR)
	cp $(BIN) $(INSTALL_DIR)/

