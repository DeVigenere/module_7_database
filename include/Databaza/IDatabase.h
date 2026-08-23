#pragma once
#include <string>

class IDatabase {
public:
    virtual ~IDatabase() = default;

    virtual bool init() = 0;
    virtual bool saveMessage(const std::string& source,
        const std::string& timestamp,
        const std::string& payload,
        long long& msgId) = 0;
    virtual void showStats() = 0;
    virtual void close() = 0;
};