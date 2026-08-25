#pragma once
#include <memory>
#include <string>
#include <cstdint>

class IConnection;

class INetworkFactory {
public:
    virtual ~INetworkFactory() = default;
    virtual bool init() = 0;
    virtual void cleanup() = 0;
    virtual std::unique_ptr<IConnection> listen(std::uint16_t port) = 0;
    virtual std::unique_ptr<IConnection> connectTo(const std::string& host, std::uint16_t port) = 0;
    virtual std::unique_ptr<IConnection> accept(std::unique_ptr<IConnection>& listeningSocket) = 0;
};