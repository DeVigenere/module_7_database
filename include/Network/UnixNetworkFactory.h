#pragma once
#include "Network/INetworkFactory.h"
#include <memory>

class UnixNetworkFactory : public INetworkFactory {
    bool initialized_ = false;
public:
    UnixNetworkFactory() = default;
    ~UnixNetworkFactory() override = default;
    bool init() override;
    void cleanup() override;
    std::unique_ptr<IListener> listen(std::uint16_t port) override;
    std::unique_ptr<IConnection> connectTo(const std::string& host, std::uint16_t port) override;
};