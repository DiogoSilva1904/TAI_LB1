# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# Executable names
FCM_EXEC = fcm
GENERATOR_EXEC = generator

# Source files
FCM_SRC = fcm.cpp
GENERATOR_SRC = generator.cpp

# Default target
all: $(FCM_EXEC) $(GENERATOR_EXEC)

# Compile fcm
$(FCM_EXEC): $(FCM_SRC)
	$(CXX) $(CXXFLAGS) -o $(FCM_EXEC) $(FCM_SRC)

# Compile generator
$(GENERATOR_EXEC): $(GENERATOR_SRC)
	$(CXX) $(CXXFLAGS) -o $(GENERATOR_EXEC) $(GENERATOR_SRC)

# Clean up compiled files
clean:
	rm -f $(FCM_EXEC) $(GENERATOR_EXEC)