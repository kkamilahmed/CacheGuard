#include "server.h"
#include "parser.h"
#include "executor.h"
#include "storage.h"
#include "ratelimiter.h"
#include "utils.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

void handleClient(SOCKET client_socket, Storage& storage, const std::string& client_ip, RateLimiter& rate_limiter) {
    std::cout << "Client connected from " << client_ip << "\n";
    logToFile("Client connected from " + client_ip);

    char buffer[512];
    int bytesReceived;
    std::string clientBuffer;
    Executor executor(storage);

    while ((bytesReceived = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        clientBuffer.append(buffer, bytesReceived);

        auto commands = Parser::parseCommands(clientBuffer);
        for (auto &cmd : commands) {
            if (!rate_limiter.allowRequest(client_ip)) {
                std::string error_msg = "-ERROR rate limit exceeded\r\n";
                send(client_socket, error_msg.c_str(), error_msg.size(), 0);
                
                if (rate_limiter.isBlocked(client_ip)) {
                    closesocket(client_socket);
                    rate_limiter.removeConnection(client_ip);
                    logToFile("Client from " + client_ip + " forcibly disconnected (blocked)");
                    return;
                }
                continue;
            }
            
            logToFile(client_ip + " executed: " + cmd);
            
            std::string reply = executor.execute(cmd);
            if (!reply.empty()) {
                send(client_socket, reply.c_str(), reply.size(), 0);
            }
        }
    }

    closesocket(client_socket);
    rate_limiter.removeConnection(client_ip);
    std::cout << "Client disconnected from " << client_ip << "\n";
    logToFile("Client disconnected from " + client_ip);
}

void startServer(int portnum) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portnum);

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(server_socket);
        WSACleanup();
        return;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(server_socket);
        WSACleanup();
        return;
    }

    std::cout << "Server listening on port " << portnum << "...\n";
    logToFile("Server started on port " + std::to_string(portnum));

    Storage storage;
    storage.load();
    
    RateLimiter rate_limiter(100, 60, 10);

    while (true) {
        sockaddr_in client_addr{};
        int client_addr_len = sizeof(client_addr);
        
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_len);
        
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::string ip_string(client_ip);

        if (!rate_limiter.allowConnection(ip_string)) {
            logToFile("Connection rejected from " + ip_string + " (rate limit or blocked)");
            closesocket(client_socket);
            continue;
        }

        std::thread(handleClient, client_socket, std::ref(storage), ip_string, std::ref(rate_limiter)).detach();
    }

    closesocket(server_socket);
    WSACleanup();
}