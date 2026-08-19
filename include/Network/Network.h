#pragma once
#include <winsock2.h>
#include <iostream>

class Network {
private:
    bool initialized = false;
public:
    bool init();
    SOCKET createServerSocket(int port);
    SOCKET acceptClient(SOCKET server_socket);
    int receive(SOCKET socket, char* buffer, int size);
    int send(SOCKET socket, const char* data, int size);
    void closeSocket(SOCKET socket);
    void cleanup();
    ~Network() {
        cleanup();
    }
};
