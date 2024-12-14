# Variables
PREFIX ?= /usr/X11R6
CXX ?= g++
CXXFLAGS ?= -Os -pedantic -Wall -Wextra -std=c++17
LDFLAGS ?= -L$(PREFIX)/lib -lX11
INCLUDES ?= -I$(PREFIX)/include
SRC = cxwm-main.cpp
BUILD_DIR = Build
TARGET = $(BUILD_DIR)/CXWM

# Reglas
all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
