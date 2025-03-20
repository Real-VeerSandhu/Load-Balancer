# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Source and Object Files
SRC = main.cpp load_balancer.cpp load_monitor.cpp server_health.cpp
OBJ = $(SRC:.cpp=.o)

# Executable
EXEC = load_balancer_system

# Include Directories (you can add directories for external libraries here if needed)
INCLUDE_DIRS = -I./

# Libraries (if any)
LIBS =

# Default target
all: $(EXEC)

# Link the executable
$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LIBS)

# Compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJ) $(EXEC)

# Rebuild everything
rebuild: clean all
