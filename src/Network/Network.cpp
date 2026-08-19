#include "Network.h"

bool Network::init() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error init WinSock" << std::endl;
        return false;
    }
    initialized = true;
    return true;
}

SOCKET Network::createServerSocket(int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Error to create socket" << std::endl;
        return INVALID_SOCKET;
    }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(sock, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Port " << port << " already used" << std::endl;
        closesocket(sock);
        return INVALID_SOCKET;
    }
    if (listen(sock, 5) == SOCKET_ERROR) {
        std::cerr << "Error listening" << std::endl;
        closesocket(sock);
        return INVALID_SOCKET;
    }
    return sock;
}

SOCKET Network::acceptClient(SOCKET server_socket) {
    sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);
    return accept(server_socket, (sockaddr*)&client_addr, &addrlen);
}

int Network::receive(SOCKET socket, char* buffer, int size) {
    return recv(socket, buffer, size, 0);
}

int Network::send(SOCKET socket, const char* data, int size) {
    return ::send(socket, data, size, 0);
}

void Network::closeSocket(SOCKET socket) {
    closesocket(socket);
}

void Network::cleanup() {
    if (initialized) {
        WSACleanup();
        initialized = false;
    }
}