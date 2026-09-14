#pragma once
#include "IDatabase.h"
#include <sqlite3.h>
#include <mutex>
#include <string>

class SqliteDatabase : public IDatabase {
private:
    sqlite3* db_ = nullptr;
    std::mutex mutex_;
    std::string dbPath_;

public:
    explicit SqliteDatabase(const std::string& path = "db/messages.db");
    ~SqliteDatabase() override;

    bool init() override;
    bool saveMessage(const std::string& source,
        const std::string& timestamp,
        const std::string& payload,
        long long& msgId) override;
    void showStats() override;
    void close() override;
};