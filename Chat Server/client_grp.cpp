// Client-side implementation in C++ for a chat server with private messages and group messaging

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

std::mutex cout_mutex;

void handle_server_messages(int server_socket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(server_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Disconnected from server." << std::endl;
            close(server_socket);
            exit(0);
        }
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << buffer << std::endl;
    }
}

int main() {
    int client_socket;
    sockaddr_in server_address{};

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Error connecting to server." << std::endl;
        return 1;
    }

    std::cout << "Connected to the server." << std::endl;

    // Authentication
    std::string username, password;
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0); // Receive the message "Enter the user name" for the server
    // You should have a line like this in the server.cpp code: send_message(client_socket, "Enter username: ");
 
    std::cout << buffer;
    std::getline(std::cin, username);
    send(client_socket, username.c_str(), username.size(), 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0); // Receive the message "Enter the password" for the server
    std::cout << buffer;
    std::getline(std::cin, password);
    send(client_socket, password.c_str(), password.size(), 0);

    memset(buffer, 0, BUFFER_SIZE);
    // Depending on whether the authentication passes or not, receive the message "Authentication Failed" or "Welcome to the server"
    recv(client_socket, buffer, BUFFER_SIZE, 0); 
    std::cout << buffer << std::endl;

    if (std::string(buffer).find("Authentication failed") != std::string::npos) {
        close(client_socket);
        return 1;
    }

    // Start thread for receiving messages from server
    std::thread receive_thread(handle_server_messages, client_socket);
    // We use detach because we want this thread to run in the background while the main thread continues running
    receive_thread.detach();

    // Send messages to the server
    while (true) {
        std::string message;
        std::getline(std::cin, message);

        if (message.empty()) continue;

        send(client_socket, message.c_str(), message.size(), 0);

        if (message == "/exit") {
            close(client_socket);
            break;
        }
    }

    return 0;
}
