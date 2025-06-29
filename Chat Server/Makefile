# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic -pthread

# Targets
SERVER_SRC = server_grp.cpp
CLIENT_SRC = client_grp.cpp
SERVER_BIN = server_grp
CLIENT_BIN = client_grp

# Default target
all: $(SERVER_BIN) $(CLIENT_BIN)

# Compile server
$(SERVER_BIN): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $(SERVER_BIN) $(SERVER_SRC)

# Compile client
$(CLIENT_BIN): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o $(CLIENT_BIN) $(CLIENT_SRC)

# Clean build artifacts
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)

