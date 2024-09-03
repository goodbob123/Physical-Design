# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -O3

# Include directories
INCLUDES = -I ./src

# Source files
SRCS = ./src/main.cpp ./src/Graph.cpp

# Header files
HEADERS = ./src/CfgJsonParser.h ./src/Astar_search.h ./src/ConnectionJsonParser.h ./src/DefParser.h ./src/Router.h ./src/Graph.h

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable
TARGET = ./bin/CGR

# Default rule
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)

# Rule to build object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
