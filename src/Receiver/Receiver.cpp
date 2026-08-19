#include "Databaza.h"
#include "Message.h"
#include "Network.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
const int PORT = 8080;
const int BUFFER_SIZE = 4096;
std::atomic<bool> running{ true };

extern Databaza g_db;
Network g_network;

DWORD WINAPI handleClient(LPVOID param) {
    SOCKET client_socket = static_cast<SOCKET>(reinterpret_cast<INT_PTR>(param));
    char buffer[BUFFER_SIZE] = { 0 };
    std::cout << "New connection" << std::endl;
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = g_network.receive(client_socket, buffer, BUFFER_SIZE - 1);
        if (bytes_read == SOCKET_ERROR) {
            if (WSAGetLastError() != WSAETIMEDOUT) {
                std::cerr << "Error to read message" << std::endl;
            }
            break;
        }
        else if (bytes_read == 0) {
            std::cout << "Client disconnect" << std::endl;
            break;
        }
        std::string received(buffer, bytes_read);
        printMessage(received);
        g_network.send(client_socket, "OK", 2);
    }
    g_network.closeSocket(client_socket);
    return 0;
}

int main() {
    if (!g_network.init()) {
        return 1;
    }
    if (!g_db.init()) {
        std::cerr << "Error init DATABAZA" << std::endl;
        g_network.cleanup();
        return 1;
    }
    SOCKET server_fd = g_network.createServerSocket(PORT);
    if (server_fd == INVALID_SOCKET) {
        g_db.close();
        g_network.cleanup();
        return 1;
    }
    std::cout << "Service works on " << PORT << std::endl;
    std::cout << "For get stats send message with source_service='control' and payload='stats'" << std::endl;
    while (running) {
        SOCKET client_socket = g_network.acceptClient(server_fd);
        if (client_socket == INVALID_SOCKET) {
            if (running) {
                std::cerr << "Error accepting" << std::endl;
            }
            continue;
        }
        std::thread thread(handleClient,
            reinterpret_cast<LPVOID>(static_cast<INT_PTR>(client_socket)));
        if (thread.joinable()) {
            thread.detach();
        }
        else {
            std::cerr << "Error to create thread" << std::endl;
            g_network.closeSocket(client_socket);
        }
    }
    g_network.closeSocket(server_fd);
    g_db.close();
    g_network.cleanup();
    std::cout << "Service was stop" << std::endl;
    system("pause");
    return 0;
}