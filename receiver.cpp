#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <map>
#include <sqlite3.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "sqlite3.lib")

const int PORT = 8080;
const int BUFFER_SIZE = 4096;
std::atomic<bool> running{ true };
sqlite3* db = nullptr;

bool initDatabase() {
    std::string dbPath = "messages.db";
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error to connect to DATABAZA: " << sqlite3_errmsg(db) << std::endl; //i know that database, but my database it's DATABAZA
        return false;
    }
    const char* createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            source_service TEXT NOT NULL,
            timestamp_utc DATETIME NOT NULL,
            payload TEXT NOT NULL,
            received_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            status TEXT DEFAULT 'received',
            schema_version INTEGER DEFAULT 1,
            processed BOOLEAN DEFAULT 0
        );
        CREATE INDEX IF NOT EXISTS idx_source_service ON messages(source_service);
        CREATE INDEX IF NOT EXISTS idx_timestamp ON messages(timestamp_utc);
        CREATE INDEX IF NOT EXISTS idx_status ON messages(status);
    )";
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Error to create DATABAZA: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    std::cout << "D" << std::endl;
    return true;
}

bool saveMessageToDB(const std::string& source, const std::string& timestamp,
    const std::string& payload, long long& msgId) {
    const char* insertSQL = R"(
        INSERT INTO messages (source_service, timestamp_utc, payload) 
        VALUES (?, ?, ?)
    )";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Preparing request: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_text(stmt, 1, source.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, timestamp.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, payload.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Insert data: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    msgId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return true;
}

void showStats() {
    const char* totalSQL = "SELECT COUNT(*) FROM messages";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, totalSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int total = sqlite3_column_int(stmt, 0);
            std::cout << "Amount messages: " << total << std::endl;
        }
        sqlite3_finalize(stmt);
    }
    const char* sourceSQL = R"(
        SELECT source_service, COUNT(*) 
        FROM messages 
        GROUP BY source_service
    )";
    if (sqlite3_prepare_v2(db, sourceSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        std::cout << "Sources:" << std::endl;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* source = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int count = sqlite3_column_int(stmt, 1);
            std::cout << "  " << (source ? source : "unknown") << ": " << count << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

bool initWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error init WinSock" << std::endl;
        return false;
    }
    return true;
}

bool isValidMessage(const std::string& msg) {
    return msg.find("\"source_service\"") != std::string::npos &&
        msg.find("\"timestamp_utc\"") != std::string::npos &&
        msg.find("\"payload\"") != std::string::npos;
}

std::string extractValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return "";
    size_t start = pos + searchKey.length();
    size_t end = json.find("\"", start);
    if (end == std::string::npos) return "";
    return json.substr(start, end - start);
}

void printMessage(const std::string& message) {
    std::cout << message << std::endl;
    if (!isValidMessage(message)) {
        std::cout << "Invalid format message" << std::endl;
        return;
    }
    std::string source = extractValue(message, "source_service");
    std::string timestamp = extractValue(message, "timestamp_utc");
    std::string payload = extractValue(message, "payload");
    std::cout << "Source: " << source << std::endl;
    std::cout << "Time: " << timestamp << std::endl;
    std::cout << "Data: " << payload << std::endl;
    if (source == "control" && payload == "stats") {
        showStats();
        return;
    }
    long long msgId = 0;
    if (saveMessageToDB(source, timestamp, payload, msgId)) {
        std::cout << "Save messages id=" << msgId << std::endl;
    }
    else {
        std::cerr << "Error to save data" << std::endl;
    }
}

DWORD WINAPI handleClient(LPVOID param) {
    SOCKET client_socket = static_cast<SOCKET>(reinterpret_cast<INT_PTR>(param));
    char buffer[BUFFER_SIZE] = { 0 };
    std::cout << "New connection" << std::endl;
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read == SOCKET_ERROR) {
            if (WSAGetLastError() != WSAETIMEDOUT) {
                std::cerr << "Error to read message" << std::endl;
            }
            break;
        }
        else if (bytes_read == 0) {
            std::cout << "Client disconnect" << std::endl;
            break;
        }
        std::string received(buffer, bytes_read);
        printMessage(received);
        const char* response = "OK";
        send(client_socket, response, strlen(response), 0);
    }
    closesocket(client_socket);
    return 0;
}

int main() {
    if (!initWinsock()) {
        return 1;
    }
    if (!initDatabase()) {
        std::cerr << "Error init DATABAZA" << std::endl;
        WSACleanup();
        return 1;
    }
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Error to create socket" << std::endl;
        sqlite3_close(db);
        WSACleanup();
        return 1;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Port " << PORT << " already used" << std::endl;
        closesocket(server_fd);
        sqlite3_close(db);
        WSACleanup();
        return 1;
    }
    if (listen(server_fd, 5) == SOCKET_ERROR) {
        std::cerr << "Error listening" << std::endl;
        closesocket(server_fd);
        sqlite3_close(db);
        WSACleanup();
        return 1;
    }
    std::cout << "Service works on " << PORT << std::endl;
    std::cout << "For get stats send message with source_service='control' and payload='stats'" << std::endl;
    while (running) {
        sockaddr_in client_addr;
        int addrlen = sizeof(client_addr);
        SOCKET client_socket = accept(server_fd, (sockaddr*)&client_addr, &addrlen);
        if (client_socket == INVALID_SOCKET) {
            if (running) {
                std::cerr << "Error accepting" << std::endl;
            }
            continue;
        }
        HANDLE thread = CreateThread(NULL, 0, handleClient,
            reinterpret_cast<LPVOID>(static_cast<INT_PTR>(client_socket)),
            0, NULL);
        if (thread != NULL) {
            CloseHandle(thread);
        }
        else {
            std::cerr << "Error to create thread" << std::endl;
            closesocket(client_socket);
        }
    }
    closesocket(server_fd);
    sqlite3_close(db);
    WSACleanup();
    std::cout << "Service was stop" << std::endl;
    system("pause");
    return 0;
}