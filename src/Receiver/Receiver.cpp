#include "Globals.h"
#include "Message.h"
#include "Network/Network.h"
#include <nlohmann/json.hpp>
#include <atomic>
#include <thread>
#include <iostream>
#include <cstring>

using json = nlohmann::json;

const int PORT = 8080;
const int BUFFER_SIZE = 4096;
std::atomic<bool> running{ true };

Network g_network;

#ifdef _WIN32
DWORD WINAPI handleClient(LPVOID param) {
    socket_t client_socket = static_cast<socket_t>(reinterpret_cast<INT_PTR>(param));
#else
void* handleClient(void* param) {
    socket_t client_socket = static_cast<socket_t>(reinterpret_cast<intptr_t>(param));
#endif
    char buffer[BUFFER_SIZE] = { 0 };
    std::cout << "New connection" << std::endl;

    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = g_network.receive(client_socket, buffer, BUFFER_SIZE - 1);
        if (bytes_read == SOCKET_ERROR_VAL) {
#ifdef _WIN32
            int error = WSAGetLastError();
            if (error != WSAETIMEDOUT && error != WSAEWOULDBLOCK) {
                std::cerr << "Error reading message: " << error << std::endl;
            }
#else
            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                std::cerr << "Error reading message: " << strerror(errno) << std::endl;
            }
#endif
            break;
        }
        else if (bytes_read == 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        }
        std::string received(buffer, bytes_read);
        printMessage(received);
        g_network.send(client_socket, "OK", 2);
    }
    g_network.closeSocket(client_socket);
#ifdef _WIN32
    return 0;
#else
    return nullptr;
#endif
}

int main() {
    if (!g_network.init()) {
        std::cerr << "Failed to initialize network" << std::endl;
        return 1;
    }
    if (!g_db->init()) {
        std::cerr << "Error initializing database" << std::endl;
        g_network.cleanup();
        return 1;
    }
    socket_t server_fd = g_network.createServerSocket(PORT);
    if (server_fd == SOCKET_INVALID_VAL) {
        g_db->close();
        g_network.cleanup();
        return 1;
    }
    std::cout << "Service running on port " << PORT << std::endl;
    std::cout << "To get stats, send message with source_service='control' and payload='stats'" << std::endl;
    while (running) {
        socket_t client_socket = g_network.acceptClient(server_fd);
        if (client_socket == SOCKET_INVALID_VAL) {
            if (running) {
                std::cerr << "Error accepting connection" << std::endl;
            }
            continue;
        }
#ifdef _WIN32
        std::thread thread(handleClient,
            reinterpret_cast<LPVOID>(static_cast<INT_PTR>(client_socket)));
#else
        std::thread thread(handleClient,
            reinterpret_cast<void*>(static_cast<intptr_t>(client_socket)));
#endif
        if (thread.joinable()) {
            thread.detach();
        }
        else {
            std::cerr << "Failed to create thread" << std::endl;
            g_network.closeSocket(client_socket);
        }
    }
    g_network.closeSocket(server_fd);
    g_db->close();
    g_network.cleanup();
    std::cout << "Service stopped" << std::endl;
    return 0;
}