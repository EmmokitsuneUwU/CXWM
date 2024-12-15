PREFIX ?= /usr/X11R6
CXX ?= g++
CXXFLAGS ?= -Os -pedantic -Wall -Wextra -std=c++17
LDFLAGS ?= -L$(PREFIX)/lib -lX11
INCLUDES ?= -I$(PREFIX)/include -I$(PWD)/Include
SRC = Src/cxwm-main.cpp
BUILD_DIR = Build
TARGET = $(BUILD_DIR)/CXWM

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	rm -rf $(BUILD_DIR)

run:
	bash runWM.sh

.PHONY: all clean
