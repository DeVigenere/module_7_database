#include "Globals.h"
#include "Message.h"
#include "Network/NetworkFactory.h"
#include "Network/IConnection.h"
#include "Network/INetworkFactory.h"
#include "Network/IListener.h"
#include <nlohmann/json.hpp>
#include <atomic>
#include <thread>
#include <iostream>
#include <memory>

using json = nlohmann::json;

const int PORT = 8080;
std::atomic<bool> running{ true };

void handleClient(std::unique_ptr<IConnection> client) {
    if (!client) {
        std::cerr << "Invalid client connection" << std::endl;
        return;
    }
    std::cout << "New connection accepted" << std::endl;
    std::string received;
    while (running && client->isOpen()) {
        std::string chunk;
        if (!client->recvSome(chunk, 1024)) {
            if (!client->isOpen()) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        received += chunk;
        size_t pos;
        while ((pos = received.find('\n')) != std::string::npos) {
            std::string message = received.substr(0, pos);
            received.erase(0, pos + 1);
            if (!message.empty()) {
                printMessage(message);
                client->sendAll("OK\n");
            }
        }
        if (received.size() > 1024) {
            std::cerr << "Message large" << std::endl;
            received.clear();
        }
    }
    client->close();
    std::cout << "Client disconnected" << std::endl;
}

int main() {
    auto factory = makeNetworkFactory();
    if (!factory || !factory->init()) {
        std::cerr << "Failed to init network" << std::endl;
        return 1;
    }
    if (!g_db->init()) {
        std::cerr << "Error init database" << std::endl;
        factory->cleanup();
        return 1;
    }
    auto listener = factory->listen(PORT);
    if (!listener) {
        g_db->close();
        factory->cleanup();
        return 1;
    }
    std::cout << "Service on port " << PORT << std::endl;
    std::cout << "To get stats, send message with source_service='control' and payload='stats'" << std::endl;
    while (running) {
        auto client = listener->accept();
        if (!client) {
            if (running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            continue;
        }
        std::thread([client = std::move(client)]() mutable {
            handleClient(std::move(client));
            }).detach();
    }
    listener->close();
    g_db->close();
    factory->cleanup();
    std::cout << "Service stop" << std::endl;
    return 0;
}