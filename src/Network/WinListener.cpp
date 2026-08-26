#include "Network/WinListener.h"
#include "Network/WinConnection.h"
#include <iostream>

WinListener::WinListener(SOCKET sock) : sock_(sock), open_(sock != INVALID_SOCKET) {}

WinListener::~WinListener() {
    close();
}

WinListener::WinListener(WinListener&& other) noexcept
    : sock_(other.sock_), open_(other.open_) {
    other.sock_ = INVALID_SOCKET;
    other.open_ = false;
}

WinListener& WinListener::operator=(WinListener&& other) noexcept {
    if (this != &other) {
        close();
        sock_ = other.sock_;
        open_ = other.open_;
        other.sock_ = INVALID_SOCKET;
        other.open_ = false;
    }
    return *this;
}

std::unique_ptr<IConnection> WinListener::accept() {
    if (!open_) {
        std::cerr << "Listener is not open" << std::endl;
        return nullptr;
    }
    SOCKET clientSock = ::accept(sock_, nullptr, nullptr);
    if (clientSock == INVALID_SOCKET) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "Accept failed: " << error << std::endl;
        }
        return nullptr;
    }
    return std::make_unique<WinConnection>(clientSock);
}

void WinListener::close() {
    if (open_ && sock_ != INVALID_SOCKET) {
        ::closesocket(sock_);
        sock_ = INVALID_SOCKET;
        open_ = false;
    }
}

bool WinListener::isOpen() const {
    return open_;
}