#pragma once
#include "Network/IConnection.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <errno.h>

class UnixConnection : public IConnection {
    int sock_;
    bool open_;
public:
    explicit UnixConnection(int s);
    ~UnixConnection() override;
    UnixConnection(const UnixConnection&) = delete;
    UnixConnection& operator=(const UnixConnection&) = delete;
    UnixConnection(UnixConnection&& other) noexcept;
    UnixConnection& operator=(UnixConnection&& other) noexcept;
    bool sendAll(std::string_view data) override;
    bool recvSome(std::string& acc, int maxBytes) override;
    void close() override;
    bool isOpen() const override;
    int getSocket() const { return sock_; }
};