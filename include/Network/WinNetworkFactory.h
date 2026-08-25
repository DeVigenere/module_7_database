#pragma once
#include "Network/INetworkFactory.h"
#include <memory>

class WinNetworkFactory : public INetworkFactory {
    bool initialized_ = false;
public:
    WinNetworkFactory() = default;
    ~WinNetworkFactory() override = default;
    bool init() override;
    void cleanup() override;
    std::unique_ptr<IConnection> listen(std::uint16_t port) override;
    std::unique_ptr<IConnection> connectTo(const std::string& host, std::uint16_t port) override;
    std::unique_ptr<IConnection> accept(std::unique_ptr<IConnection>& listeningSocket) override;
};