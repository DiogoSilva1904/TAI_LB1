# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# Executable name
FCM_EXEC = fcm

# Source file
FCM_SRC = fcm.cpp

# Default target
all: $(FCM_EXEC)

# Compile fcm
$(FCM_EXEC): $(FCM_SRC)
	$(CXX) $(CXXFLAGS) -o $(FCM_EXEC) $(FCM_SRC)

# Clean up compiled files
clean:
	rm -f $(FCM_EXEC)
