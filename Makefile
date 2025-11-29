CXX = clang++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -g -fsanitize=address
LDFLAGS = -fsanitize=address
SRCDIR = src
BUILDDIR = build
TARGET = sim

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

# Default target
all: $(BUILDDIR) $(TARGET)

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Link object files into executable
$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(TARGET)
	@echo "Build complete! Run ./$(TARGET) to start the simulator."

# Compile source files to object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR) $(TARGET)
	@echo "Clean complete."

# Phony targets
.PHONY: all clean

