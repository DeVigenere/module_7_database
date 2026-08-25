#pragma once
#include "Network/IConnection.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class WinConnection : public IConnection {
private:
    SOCKET sock_;
    bool open_;
public:
    explicit WinConnection(SOCKET s);
    ~WinConnection() override;
    WinConnection(const WinConnection&) = delete;
    WinConnection& operator=(const WinConnection&) = delete;
    WinConnection(WinConnection&& other) noexcept;
    WinConnection& operator=(WinConnection&& other) noexcept;
    bool sendAll(std::string_view data) override;
    bool recvSome(std::string& acc, int maxBytes) override;
    void close() override;
    bool isOpen() const override;
    SOCKET getSocket() const { return sock_; }
};