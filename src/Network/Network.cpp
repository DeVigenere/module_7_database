#include "Network.h"

Network::~Network() {
    cleanup();
}

bool Network::init() {
    if (initialized) return true;
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }
#endif
    initialized = true;
    return true;
}
void Network::cleanup() {
    if (initialized) {
#ifdef _WIN32
        WSACleanup();
#endif
        initialized = false;
    }
}

socket_t Network::createServerSocket(int port) {
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == SOCKET_INVALID_VAL) {
        std::cerr << "Failed to create socket" << std::endl;
        return SOCKET_INVALID_VAL;
    }
    int opt = 1;
#ifdef _WIN32
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) != 0) {
#else
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
#endif
        std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
        SOCKET_CLOSE_FN(sock);
        return SOCKET_INVALID_VAL;
    }
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(sock, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR_VAL) {
        std::cerr << "Failed to bind to port " << port << std::endl;
        SOCKET_CLOSE_FN(sock);
        return SOCKET_INVALID_VAL;
    }

    if (listen(sock, 5) == SOCKET_ERROR_VAL) {
        std::cerr << "Failed to listen on port " << port << std::endl;
        SOCKET_CLOSE_FN(sock);
        return SOCKET_INVALID_VAL;
    }
    std::cout << "Server socket created on port " << port << std::endl;
    return sock;
    }
socket_t Network::acceptClient(socket_t server_socket) {
    sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    socket_t client_socket = accept(server_socket, (sockaddr*)&client_addr, &addrlen);

    if (client_socket == SOCKET_INVALID_VAL) {
#ifdef _WIN32
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "Accept failed: " << error << std::endl;
        }
#else
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        }
#endif
    }
    return client_socket;
}
int Network::receive(socket_t socket, char* buffer, int size) {
    if (!buffer || size <= 0) return -1;
    return recv(socket, buffer, size, 0);
}
int Network::send(socket_t socket, const char* data, int size) {
    if (!data || size <= 0) return -1;
    return ::send(socket, data, size, 0);
}
void Network::closeSocket(socket_t socket) {
    if (socket != SOCKET_INVALID_VAL) {
        SOCKET_CLOSE_FN(socket);
    }
}
bool Network::isInitialized() const {
    return initialized;
}