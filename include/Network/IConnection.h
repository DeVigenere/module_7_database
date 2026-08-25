#pragma once
#include <string>

class IConnection {
public:
    virtual ~IConnection() = default;
    virtual bool sendAll(std::string_view data) = 0;
    virtual bool recvSome(std::string& acc, int maxBytes) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
};