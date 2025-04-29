#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

SOCKET sockfd;
bool running = true;

void receive_messages() {
    char buffer[BUFFER_SIZE];
    int recv_size;

    while (running && (recv_size = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
        buffer[recv_size] = '\0';
        std::cout << buffer << std::endl;
    }
}

void send_messages() {
    char message[BUFFER_SIZE];

    while (running) {
        std::cin.getline(message, sizeof(message));
        std::string msg_str(message);

        if (msg_str == "/exit") {
            send(sockfd, message, strlen(message), 0);
            running = false;
            break;
        }

        send(sockfd, message, strlen(message), 0);
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    sockaddr_in server_addr;
    char username[BUFFER_SIZE];

    std::cout << "Enter your username: ";
    std::cin >> username;
    std::cin.ignore();

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sockfd, (sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed.\n";
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    send(sockfd, username, strlen(username), 0);

    std::thread(receive_messages).detach();
    send_messages();

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
