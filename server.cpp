#include <ctime>
#include <iomanip>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

std::string current_timestamp() {
    auto now = std::time(nullptr);
    std::tm tm{};
    localtime_s(&tm, &now);
    std::ostringstream oss;
    oss << "[" << std::put_time(&tm, "%H:%M:%S") << "]";
    return oss.str();
}

struct Client {
    SOCKET sockfd;
    std::string username;
    sockaddr_in address;
    std::string group = "";
};

std::vector<Client> clients;
std::mutex clients_mutex;

void broadcast_message(const std::string &message, SOCKET sender_sockfd) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto &client : clients) {
        if (client.sockfd != sender_sockfd) {
            send(client.sockfd, message.c_str(), message.length(), 0);
        }
    }
}
void send_private_message(const std::string& recipient, const std::string& message, SOCKET sender_sockfd) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& client : clients) {
        if (client.username == recipient) {
            send(client.sockfd, message.c_str(), message.length(), 0);
            return;
        }
    }
    // Optional: notify sender that user was not found
    std::string error_msg = current_timestamp() + " User '" + recipient + "' not found.";
    send(sender_sockfd, error_msg.c_str(), error_msg.length(), 0);
}

void send_group_message(const std::string& group, const std::string& message, SOCKET sender_sockfd) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& client : clients) {
        if (client.sockfd != sender_sockfd && client.group == group) {
            send(client.sockfd, message.c_str(), message.length(), 0);
        }
    }
}



void handle_client(Client client) {
    char buffer[BUFFER_SIZE];
    int recv_size;

    while ((recv_size = recv(client.sockfd, buffer, sizeof(buffer), 0)) > 0) {
        buffer[recv_size] = '\0';
        std::string msg_str(buffer);

        if (msg_str.rfind("/msg ", 0) == 0) {
            size_t first_space = msg_str.find(' ', 5);
            if (first_space != std::string::npos) {
                std::string recipient = msg_str.substr(5, first_space - 5);
                std::string private_text = msg_str.substr(first_space + 1);
                std::string private_msg = current_timestamp() + " [Private] " + client.username + ": " + private_text;
                send_private_message(recipient, private_msg, client.sockfd);
            } else {
                std::string usage = current_timestamp() + " Usage: /msg <username> <message>";
                send(client.sockfd, usage.c_str(), usage.length(), 0);
            }
            continue;
        }
        if (msg_str == "/users") {
            std::lock_guard<std::mutex> lock(clients_mutex);
            std::string user_list = current_timestamp() + " Online users: ";
            for (size_t i = 0; i < clients.size(); ++i) {
                user_list += clients[i].username;
                if (i != clients.size() - 1)
                    user_list += ", ";
            }
            send(client.sockfd, user_list.c_str(), user_list.length(), 0);
            continue;
        }
        // /create <groupname>
if (msg_str.rfind("/create ", 0) == 0) {
    std::string group_name = msg_str.substr(8);
    client.group = group_name;
    std::string confirmation = current_timestamp() + " Created and joined group: " + group_name;
    send(client.sockfd, confirmation.c_str(), confirmation.length(), 0);
    continue;
}

// /join <groupname>
if (msg_str.rfind("/join ", 0) == 0) {
    std::string group_name = msg_str.substr(6);
    client.group = group_name;
    std::string confirmation = current_timestamp() + " Joined group: " + group_name;
    send(client.sockfd, confirmation.c_str(), confirmation.length(), 0);
    continue;
}

// /leave
if (msg_str == "/leave") {
    client.group = "";
    std::string confirmation = current_timestamp() + " You left the group.";
    send(client.sockfd, confirmation.c_str(), confirmation.length(), 0);
    continue;
}

// /gmsg <message>
if (msg_str.rfind("/gmsg ", 0) == 0) {
    if (client.group.empty()) {
        std::string warning = current_timestamp() + " You are not in a group.";
        send(client.sockfd, warning.c_str(), warning.length(), 0);
    } else {
        std::string gmsg = msg_str.substr(6);
        std::string full_msg = current_timestamp() + " [Group: " + client.group + "] " + client.username + ": " + gmsg;
        send_group_message(client.group, full_msg, client.sockfd);
    }
    continue;
}

if (msg_str == "/help") {
    std::string help_text =
        current_timestamp() + " Available commands:\n"
        "/exit                 - Leave the chat\n"
        "/users                - List all online users\n"
        "/msg <user> <msg>     - Send private message\n"
        "/create <group>       - Create and join a group\n"
        "/join <group>         - Join an existing group\n"
        "/leave                - Leave current group\n"
        "/gmsg <msg>           - Send message to current group\n"
        "/help                 - Show this help menu\n";
    send(client.sockfd, help_text.c_str(), help_text.length(), 0);
    continue;
}



        if (msg_str == "/exit") {
            std::string leave_msg = current_timestamp() + " " + client.username + " has left the chat.";

            broadcast_message(leave_msg, client.sockfd);
            break;
        }

        std::string message = current_timestamp() + " " + client.username + ": " + msg_str;
        broadcast_message(message, client.sockfd);
    }

    std::lock_guard<std::mutex> lock(clients_mutex);
    clients.erase(std::remove_if(clients.begin(), clients.end(),
                                 [client](const Client &c) { return c.sockfd == client.sockfd; }),
                  clients.end());

    closesocket(client.sockfd);
}

void start_server() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }

    SOCKET server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return;
    }

    sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sockfd, (sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        closesocket(server_sockfd);
        WSACleanup();
        return;
    }

    if (listen(server_sockfd, MAX_CLIENTS) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        closesocket(server_sockfd);
        WSACleanup();
        return;
    }

    std::cout << "Server started on port " << PORT << "..." << std::endl;

    while (true) {
        SOCKET new_sockfd = accept(server_sockfd, (sockaddr *)&client_addr, &client_addr_len);
        if (new_sockfd == INVALID_SOCKET) {
            std::cerr << "Accept failed.\n";
            continue;
        }

        char username[BUFFER_SIZE];
        recv(new_sockfd, username, sizeof(username), 0);
        std::string username_str = username;

        Client new_client = {new_sockfd, username_str, client_addr};
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(new_client);
        }

        std::cout << username_str << " connected." << std::endl;
        std::thread(handle_client, new_client).detach();
    }
    

    closesocket(server_sockfd);
    WSACleanup();
}



int main() {
    start_server();
    return 0;
}
