#include "WinConnection.h"
#include <iostream>
#include <cstring>

WinConnection::WinConnection(SOCKET s) : sock_(s), open_(s != INVALID_SOCKET) {}

WinConnection::~WinConnection() {
    close();
}

WinConnection::WinConnection(WinConnection&& other) noexcept
    : sock_(other.sock_), open_(other.open_) {
    other.sock_ = INVALID_SOCKET;
    other.open_ = false;
}

WinConnection& WinConnection::operator=(WinConnection&& other) noexcept {
    if (this != &other) {
        close();
        sock_ = other.sock_;
        open_ = other.open_;
        other.sock_ = INVALID_SOCKET;
        other.open_ = false;
    }
    return *this;
}

bool WinConnection::sendAll(std::string_view data) {
    if (!open_) {
        std::cerr << "socket is not open" << std::endl;
        return false;
    }
    int totalSent = 0;
    int remaining = static_cast<int>(data.size());
    while (remaining > 0) {
        int sent = ::send(sock_, data.data() + totalSent, remaining, 0);
        if (sent == SOCKET_ERROR) {
            std::cerr << "send failed: " << WSAGetLastError() << std::endl;
            return false;
        }
        totalSent += sent;
        remaining -= sent;
    }
    return true;
}

bool WinConnection::recvSome(std::string& acc, int maxBytes) {
    if (!open_) {
        std::cerr << "socket is not open" << std::endl;
        return false;
    }
    char buffer[1024];
    int bytesToRead = (maxBytes > 0 && maxBytes < 1024) ? maxBytes : 1024;
    int bytesRead = ::recv(sock_, buffer, bytesToRead, 0);
    if (bytesRead == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK && error != WSAETIMEDOUT) {
            std::cerr << "recv failed: " << error << std::endl;
            open_ = false;
        }
        return false;
    }
    if (bytesRead == 0) {
        open_ = false;
        return false;
    }
    acc.append(buffer, bytesRead);
    return true;
}

void WinConnection::close() {
    if (open_ && sock_ != INVALID_SOCKET) {
        ::closesocket(sock_);
        sock_ = INVALID_SOCKET;
        open_ = false;
    }
}

bool WinConnection::isOpen() const {
    return open_;
}