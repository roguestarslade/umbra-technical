# Makefile for propulsion TCP server (header-only implementation)

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

TARGET = propulsion_server
SRC = main.cpp

DEPS = \
	TCPServer.hpp \
	CommandTimer.hpp \
	StatusDisplay.hpp

$(TARGET): $(SRC) $(DEPS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)