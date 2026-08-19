#pragma once
#include <string>
#include <mutex>
#include "sqlite3/sqlite3.h"

class Databaza {
private:
    sqlite3* db_ = nullptr;
    std::mutex mutex_;
public:
    Databaza() = default;
    ~Databaza();

    Databaza(const Databaza&) = delete;
    Databaza& operator=(const Databaza&) = delete;

    bool init();
    bool saveMessage(const std::string& source,
        const std::string& timestamp,
        const std::string& payload,
        long long& msgId);
    void showStats();
    void close();

};