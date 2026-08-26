#pragma once
#include "Network/IListener.h"
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>

class UnixListener : public IListener {
    int sock_;
    bool open_;
public:
    explicit UnixListener(int sock);
    ~UnixListener() override;
    UnixListener(const UnixListener&) = delete;
    UnixListener& operator=(const UnixListener&) = delete;
    UnixListener(UnixListener&& other) noexcept;
    UnixListener& operator=(UnixListener&& other) noexcept;
    std::unique_ptr<IConnection> accept() override;
    void close() override;
    bool isOpen() const override;
};