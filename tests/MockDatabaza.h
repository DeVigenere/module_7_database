#pragma once
#include "Databaza/IDatabase.h"
#include <vector>
#include <iostream>

struct TestMessage {
    std::string source;
    std::string timestamp;
    std::string payload;
    long long id;
};

class MockDatabase : public IDatabase {
private:
    std::vector<TestMessage> messages_;
    long long nextId_ = 1;
    bool statsCalled_ = false;
public:
    bool init() override {
        std::cout << "DATABAZA INIT" << std::endl;
        return true;
    }
    bool saveMessage(const std::string& source,
        const std::string& timestamp,
        const std::string& payload,
        long long& msgId) override {
        messages_.push_back({ source, timestamp, payload, nextId_ });
        msgId = nextId_++;
        std::cout << "Mock save: " << source << " -> " << payload << std::endl;
        return true;
    }
    void showStats() override {
        statsCalled_ = true;
        std::cout << "=== MOCK STATS ===" << std::endl;
        std::cout << "Total: " << messages_.size() << " messages" << std::endl;
        for (const auto& msg : messages_) {
            std::cout << "  " << msg.source << ": " << msg.payload << std::endl;
        }
    }
    void close() override {
        std::cout << "Mock DB closed" << std::endl;
    }
    const auto& getMessages() const { return messages_; }
    bool wasStatsCalled() const { return statsCalled_; }
    void resetStats() { statsCalled_ = false; }
};