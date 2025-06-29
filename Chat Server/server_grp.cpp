#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024
std::mutex client_mutex;
std::unordered_map<int, std::string> clients;
std::unordered_map<std::string, std::string> users;
std::unordered_map<std::string, std::unordered_set<int>> groups;

void load_users() { //loading the username and password data from users.txt file into users array
    std::ifstream file("users.txt");
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find(":");
        if (pos != std::string::npos) {
            std::string username = line.substr(0, pos);
            std::string password = line.substr(pos + 1);

            username.erase(username.find_last_not_of("\r\n") + 1);
            password.erase(password.find_last_not_of("\r\n") + 1);

            users[username] = password;
        }
    }
}


void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::string username;

    send(client_socket, "Enter username: ", 17, 0); //sending Client socket message to Enter Username
    recv(client_socket, buffer, BUFFER_SIZE, 0); //Recieving input username from client
    username = std::string(buffer);
    username.erase(username.find_last_not_of("\r\n") + 1);

    send(client_socket, "Enter password: ", 17, 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0); //recieving input password from client
    std::string password(buffer);
    password.erase(password.find_last_not_of("\r\n") + 1);


    if (users.find(username) == users.end() || users[username] != password) {
        send(client_socket, "Authentication failed", 22, 0);
        close(client_socket);
        return;
    }
    
    send(client_socket, "Welcome to the chat server!", 26, 0); //welcome message to client in case of successfull authentication
    clients[client_socket]=username; //loading active users in clients 
    
    std::string welcome_mssg=username+" has joined the chat."; 

    for(auto& [sock, user] : clients){  // informing other active users of the current user joining the server
        if(user!=username){
            send(sock,welcome_mssg.c_str(),welcome_mssg.size(),0);
        }
    }


    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);  //recieving command from client socket
        if (bytes_received <= 0) break;
        std::string message(buffer);
        

        if (message.rfind("/broadcast ", 0) == 0) {  //processing client command for a broadcast message
            std::lock_guard<std::mutex> lock(client_mutex);
            std::string msg_content=message.substr(11);
            msg_content=username+" : "+msg_content;
            for (auto& [sock, _] : clients) {
                if (sock != client_socket) send(sock, msg_content.c_str(), msg_content.size(), 0);
            }
        }


        if (message.rfind("/msg ", 0) == 0) {  //processing client command for private message
            size_t space = message.find(' ', 5);
            if (space != std::string::npos) {
                std::string recipient = message.substr(5, space - 5);
                std::string msg_content = message.substr(space + 1);
                msg_content=username+" : "+msg_content;
                for (auto& [sock, user] : clients) {
                    if (user == recipient) {
                        send(sock, msg_content.c_str(), msg_content.size(), 0);
                        break;
                    }
                }
            }
        }

        std::string grp_welcome;
        if (message.rfind("/create_grp ", 0) == 0) { //Creating group if any such command recieved
            std::string group = message.substr(12);
            groups[group] = {client_socket};
            grp_welcome="Group "+group+" created.";
            send(client_socket,grp_welcome.c_str(),grp_welcome.size(),0); //send successfull group creation message
        }


        if (message.rfind("/join_grp ", 0) == 0) {  //join group if commanded so
            std::string group = message.substr(10);
            groups[group].insert(client_socket);
            grp_welcome="You joined the group "+group;
            send(client_socket,grp_welcome.c_str(),grp_welcome.size(),0);
            grp_welcome="["+group+"]: "+username+" has joined the group.";
            for (int sock : groups[group]) {  //informing other group users of current user joining the group
                if (sock != client_socket) {
                    send(sock, grp_welcome.c_str(), grp_welcome.size(), 0);
                }
            }
        }


        if (message.rfind("/leave_grp ", 0) == 0) {  //exit group feature
            std::string group = message.substr(10);
            groups[group].erase(client_socket);
            grp_welcome="You left the group "+group;
            send(client_socket,grp_welcome.c_str(),grp_welcome.size(),0);
            grp_welcome="["+group+"]: "+username+" has left the group.";
            for (int sock : groups[group]) {
                if (sock != client_socket) {
                    send(sock, grp_welcome.c_str(), grp_welcome.size(), 0);
                }
            }
        }


        if (message.rfind("/group_msg ", 0) == 0) {  //group message implementation
            size_t space = message.find(' ', 11);
            if (space != std::string::npos) {
                std::string group = message.substr(11, space - 11);
                std::string msg_content = message.substr(space + 1);
                msg_content="["+group+"]: "+username+" : "+msg_content;
                if (groups.find(group) != groups.end() && groups[group].count(client_socket)) {
                    for (int sock : groups[group]) {
                        if (sock != client_socket) {
                            send(sock, msg_content.c_str(), msg_content.size(), 0);
                        }
                    }
                }
            }
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(client_mutex);
        clients.erase(client_socket);
    }
    close(client_socket);
}


int main() {
    load_users();
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);  //creating server socket
    sockaddr_in address{AF_INET, htons(PORT), INADDR_ANY};
    bind(server_fd, (sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);
    while (true) {
        int client_socket = accept(server_fd, nullptr, nullptr);  
        std::thread(handle_client, client_socket).detach(); //handling multiple client users in the server simultaneoulsy
    }
}