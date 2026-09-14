#pragma once
#include "Network/IListener.h"
#include <winsock2.h>
#include <memory>

class WinListener : public IListener {
    SOCKET sock_;
    bool open_;
public:
    explicit WinListener(SOCKET sock);
    ~WinListener() override;
    WinListener(const WinListener&) = delete;
    WinListener& operator=(const WinListener&) = delete;
    WinListener(WinListener&& other) noexcept;
    WinListener& operator=(WinListener&& other) noexcept;
    std::unique_ptr<IConnection> accept() override;
    void close() override;
    bool isOpen() const override;
};