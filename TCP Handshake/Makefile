# Compiler
CXX = g++
CXXFLAGS = -Wall -std=c++17

# Targets
TARGETS = server client

# Build rules
all: $(TARGETS)

server: server.cpp
	$(CXX) $(CXXFLAGS) server.cpp -o server

client: client.cpp
	$(CXX) $(CXXFLAGS) client.cpp -o client

# Clean rule
clean:
	rm -f $(TARGETS)

# Run server
run-server: server
	./server

# Run client
run-client: client
	./client

