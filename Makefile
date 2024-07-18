# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++17

# Source files
SRCS = main.cpp cpu.cpp ppu.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = nemu

# Default target
all: $(EXEC)

# Rule to link object files into the executable
$(EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $(EXEC)

# Rule to compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJS) $(EXEC)

# Phony targets
.PHONY: all clean
