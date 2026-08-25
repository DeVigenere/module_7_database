#include "Network/UnixNetworkFactory.h"
#include "Network/UnixConnection.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

bool UnixNetworkFactory::init() {
    if (initialized_) return true;
    initialized_ = true;
    return true;
}

void UnixNetworkFactory::cleanup() {
    initialized_ = false;
}

std::unique_ptr<IConnection> UnixNetworkFactory::listen(std::uint16_t port) {
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed create socket: " << strerror(errno) << std::endl;
        return nullptr;
    }
    int opt = 1;
    if (::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        std::cerr << "Failed to set SO_REUSEADDR: " << strerror(errno) << std::endl;
        close(sock);
        return nullptr;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (::bind(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        std::cerr << "Failed bind to port " << port << ": " << strerror(errno) << std::endl;
        close(sock);
        return nullptr;
    }
    if (::listen(sock, SOMAXCONN) != 0) {
        std::cerr << "Failed listen: " << strerror(errno) << std::endl;
        close(sock);
        return nullptr;
    }
    std::cout << "Listening on port " << port << std::endl;
    return std::make_unique<UnixConnection>(sock);
}

std::unique_ptr<IConnection> UnixNetworkFactory::connectTo(const std::string& host, std::uint16_t port) {
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed create socket: " << strerror(errno) << std::endl;
        return nullptr;
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) {
        std::cerr << "Invalid address: " << host << std::endl;
        close(sock);
        return nullptr;
    }
    if (::connect(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        std::cerr << "Failed connect to " << host << ":" << port << " - " << strerror(errno) << std::endl;
        close(sock);
        return nullptr;
    }
    return std::make_unique<UnixConnection>(sock);
}

std::unique_ptr<IConnection> UnixNetworkFactory::accept(std::unique_ptr<IConnection>& listeningSocket) {
    auto* unixConn = dynamic_cast<UnixConnection*>(listeningSocket.get());
    if (!unixConn) {
        std::cerr << "Invalid listening socket type" << std::endl;
        return nullptr;
    }
    int clientSock = ::accept(unixConn->getSocket(), nullptr, nullptr);
    if (clientSock < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "failed: " << strerror(errno) << std::endl;
        }
        return nullptr;
    }

    return std::make_unique<UnixConnection>(clientSock);
}