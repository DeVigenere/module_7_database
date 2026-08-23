#include "Calculator.h"
#include "Network/Network.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

using json = nlohmann::json;

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

std::string buildMessage(const std::string& source, const std::string& payload) {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf;
    std::stringstream ss;
#ifdef _WIN32
    if (gmtime_s(&tm_buf, &now_c) == 0) {
        ss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
    }
#else
    if (gmtime_r(&now_c, &tm_buf) != nullptr) {
        ss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
    }
#endif
    else {
        ss << "1970-01-01T00:00:00Z";
    }
    json messageJson;
    messageJson["source_service"] = source;
    messageJson["timestamp_utc"] = ss.str();
    messageJson["payload"] = payload;
    return messageJson.dump();
}

std::string catchfullrecv(socket_t sock) {
    std::string response;
    char buffer[BUFFER_SIZE] = { 0 };
    int timeout = 2000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    while (true) {
        int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes > 0) {
            response += std::string(buffer, bytes);
        }
        else {
            break;
        }
    }
    if (!response.empty()) {
        std::cout << "Server response: " << response << std::endl;
    }
    return response;
}

bool sendMessage(const std::string& message) {
    Network net;
    if (!net.init()) {
        std::cerr << "Failed to initialize network" << std::endl;
        return false;
    }
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == SOCKET_INVALID_VAL) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
#ifdef _WIN32
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#else
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
#endif
    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR_VAL) {
        std::cerr << "Failed to connect to server" << std::endl;
        net.closeSocket(sock);
        return false;
    }
    auto bytes_sent = net.send(sock, message.c_str(), static_cast<int>(message.length()));
    if (bytes_sent == SOCKET_ERROR_VAL) {
        std::cerr << "Failed to send message" << std::endl;
        net.closeSocket(sock);
        return false;
    }
    catchfullrecv(sock);
    net.closeSocket(sock);
    net.cleanup();
    return true;
}

int main() {
    Calculator calc;
    struct Operation {
        int a, b;
        char op;
    };
    Operation ops[] = {
        {10, 5, '+'},
        {20, 4, '-'},
        {7, 3, '*'},
        {15, 3, '/'},
        {8, 0, '/'}
    };
    int sent_count = 0;
    int total_ops = sizeof(ops) / sizeof(ops[0]);
    for (const auto& op : ops) {
        std::string result;
        try {
            switch (op.op) {
            case '+':
                result = std::to_string(calc.Add(op.a, op.b));
                break;
            case '-':
                result = std::to_string(calc.Subtract(op.a, op.b));
                break;
            case '*':
                result = std::to_string(calc.Multiply(op.a, op.b));
                break;
            case '/':
                result = std::to_string(calc.Divide(op.a, op.b));
                break;
            }
            std::string payload = std::to_string(op.a) + " " + op.op + " " +
                std::to_string(op.b) + " = " + result;
            std::string message = buildMessage("calculator", payload);
            std::cout << "\nSending: " << message << std::endl;
            if (sendMessage(message)) {
                sent_count++;
            }
            else {
                std::cerr << "Failed to send message" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Calculation error: " << e.what() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "\nSent " << sent_count << " out of " << total_ops << " messages" << std::endl;
    return 0;
}