#include "Message.h"
#include "Globals.h"

bool parseMessage(const std::string& jsonStr, Message& msg) {
    try {
        json j = json::parse(jsonStr);
        if (!j.contains("source_service") || !j.contains("timestamp_utc") || !j.contains("payload")) {
            return false;
        }
        msg.source_service = j["source_service"].get<std::string>();
        msg.timestamp_utc = j["timestamp_utc"].get<std::string>();
        msg.payload = j["payload"].get<std::string>();
        if (j.contains("status")) {
            msg.status = j["status"].get<std::string>();
        }
        if (j.contains("schema_version")) {
            msg.schema_version = j["schema_version"].get<int>();
        }
        if (j.contains("processed")) {
            msg.processed = j["processed"].get<bool>();
        }
        return true;
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
}

void printMessage(const std::string& messageStr) {
    std::cout << "Received: " << messageStr << std::endl;
    Message msg;
    if (!parseMessage(messageStr, msg)) {
        std::cout << "Invalid message format" << std::endl;
        return;
    }
    std::cout << "Source: " << msg.source_service << std::endl;
    std::cout << "Time: " << msg.timestamp_utc << std::endl;
    std::cout << "Data: " << msg.payload << std::endl;
    if (msg.status.has_value()) {
        std::cout << "Status: " << msg.status.value() << std::endl;
    }
    if (msg.schema_version.has_value()) {
        std::cout << "Schema version: " << msg.schema_version.value() << std::endl;
    }
    if (msg.processed.has_value()) {
        std::cout << "Processed: " << (msg.processed.value() ? "true" : "false") << std::endl;
    }
    if (msg.source_service == "control" && msg.payload == "stats") {
        g_db->showStats();
        return;
    }
    long long msgId = 0;
    if (g_db->saveMessage(msg.source_service, msg.timestamp_utc, msg.payload, msgId)) {
        std::cout << "Message saved with id=" << msgId << std::endl;
    }
    else {
        std::cerr << "Error saving message" << std::endl;
    }
}