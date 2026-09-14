#pragma once
#include <memory>
#include <string>
#include <cstdint>

class IConnection;
class IListener;

class INetworkFactory {
public:
    virtual ~INetworkFactory() = default;
    virtual bool init() = 0;
    virtual void cleanup() = 0;
    virtual std::unique_ptr<IListener> listen(std::uint16_t port) = 0;
    virtual std::unique_ptr<IConnection> connectTo(const std::string& host, std::uint16_t port) = 0;
};