#include "Network/UnixListener.h"
#include "Network/UnixConnection.h"
#include <iostream>
#include <unistd.h>
#include <errno.h>

UnixListener::UnixListener(int sock) : sock_(sock), open_(sock >= 0) {}

UnixListener::~UnixListener() {
    close();
}

UnixListener::UnixListener(UnixListener&& other) noexcept
    : sock_(other.sock_), open_(other.open_) {
    other.sock_ = -1;
    other.open_ = false;
}

UnixListener& UnixListener::operator=(UnixListener&& other) noexcept {
    if (this != &other) {
        close();
        sock_ = other.sock_;
        open_ = other.open_;
        other.sock_ = -1;
        other.open_ = false;
    }
    return *this;
}

std::unique_ptr<IConnection> UnixListener::accept() {
    if (!open_) {
        std::cerr << "Listener is not open" << std::endl;
        return nullptr;
    }

    int clientSock = ::accept(sock_, nullptr, nullptr);
    if (clientSock < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        }
        return nullptr;
    }
    return std::make_unique<UnixConnection>(clientSock);
}

void UnixListener::close() {
    if (open_ && sock_ >= 0) {
        ::close(sock_);
        sock_ = -1;
        open_ = false;
    }
}

bool UnixListener::isOpen() const {
    return open_;
}