#include "Network/UnixConnection.h"
#include <iostream>
#include <unistd.h>

UnixConnection::UnixConnection(int s) : sock_(s), open_(s >= 0) {}

UnixConnection::~UnixConnection() {
    close();
}

UnixConnection::UnixConnection(UnixConnection&& other) noexcept
    : sock_(other.sock_), open_(other.open_) {
    other.sock_ = -1;
    other.open_ = false;
}

UnixConnection& UnixConnection::operator=(UnixConnection&& other) noexcept {
    if (this != &other) {
        close();
        sock_ = other.sock_;
        open_ = other.open_;
        other.sock_ = -1;
        other.open_ = false;
    }
    return *this;
}

bool UnixConnection::sendAll(std::string_view data) {
    if (!open_) {
        std::cerr << "socket is not open" << std::endl;
        return false;
    }

    int totalSent = 0;
    int remaining = static_cast<int>(data.size());

    while (remaining > 0) {
        int sent = ::send(sock_, data.data() + totalSent, remaining, 0);
        if (sent < 0) {
            std::cerr << "send failed: " << strerror(errno) << std::endl;
            return false;
        }
        totalSent += sent;
        remaining -= sent;
    }
    return true;
}

bool UnixConnection::recvSome(std::string& acc, int maxBytes) {
    if (!open_) {
        std::cerr << "socket is not open" << std::endl;
        return false;
    }

    char buffer[4096];
    int bytesToRead = (maxBytes > 0 && maxBytes < 4096) ? maxBytes : 4096;
    int bytesRead = ::recv(sock_, buffer, bytesToRead, 0);
    if (bytesRead < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            std::cerr << "recv failed: " << strerror(errno) << std::endl;
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

void UnixConnection::close() {
    if (open_ && sock_ >= 0) {
        ::close(sock_);
        sock_ = -1;
        open_ = false;
    }
}

bool UnixConnection::isOpen() const {
    return open_;
}