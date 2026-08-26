#pragma once
#include <memory>

class IConnection;

class IListener {
public:
    virtual ~IListener() = default;
    virtual std::unique_ptr<IConnection> accept() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
};