#include "WinNetworkFactory.h"
#include "WinConnection.h"
#include <iostream>
#include <cstring>

bool WinNetworkFactory::init() {
    if (initialized_) return true;
    WSADATA wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }
    initialized_ = true;
    return true;
}

void WinNetworkFactory::cleanup() {
    if (initialized_) {
        ::WSACleanup();
        initialized_ = false;
    }
}

std::unique_ptr<IConnection> WinNetworkFactory::listen(std::uint16_t port) {
    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed create socket: " << WSAGetLastError() << std::endl;
        return nullptr;
    }
    int opt = 1;
    if (::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt)) != 0) {
        std::cerr << "Failed to set SO_REUSEADDR: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        return nullptr;
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (::bind(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Failed bind to port " << port << ": " << WSAGetLastError() << std::endl;
        closesocket(sock);
        return nullptr;
    }
    if (::listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Failed listen: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        return nullptr;
    }
    std::cout << "Listening on port " << port << std::endl;
    return std::make_unique<WinConnection>(sock);
}

std::unique_ptr<IConnection> WinNetworkFactory::connectTo(const std::string& host, std::uint16_t port) {
    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return nullptr;
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) {
        std::cerr << "Invalid address: " << host << std::endl;
        closesocket(sock);
        return nullptr;
    }
    if (::connect(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect " << host << ":" << port << " - " << WSAGetLastError() << std::endl;
        closesocket(sock);
        return nullptr;
    }
    return std::make_unique<WinConnection>(sock);
}

std::unique_ptr<IConnection> WinNetworkFactory::accept(std::unique_ptr<IConnection>& listeningSocket) {
    auto* winConn = dynamic_cast<WinConnection*>(listeningSocket.get());
    if (!winConn) {
        std::cerr << "Invalid listening socket" << std::endl;
        return nullptr;
    }
    SOCKET clientSock = ::accept(winConn->getSocket(), nullptr, nullptr);
    if (clientSock == INVALID_SOCKET) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "Accept failed: " << error << std::endl;
        }
        return nullptr;
    }
    return std::make_unique<WinConnection>(clientSock);
}