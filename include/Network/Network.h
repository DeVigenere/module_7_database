#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using socket_t = SOCKET;
#define SOCKET_INVALID_VAL INVALID_SOCKET
#define SOCKET_ERROR_VAL SOCKET_ERROR
#define SOCKET_CLOSE_FN closesocket
#define SOCKET_GET_LAST_ERROR WSAGetLastError()
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>

using socket_t = int;
#define SOCKET_INVALID_VAL (-1)
#define SOCKET_ERROR_VAL (-1)
#define SOCKET_CLOSE_FN close
#define SOCKET_GET_LAST_ERROR errno
#endif

#include <iostream>
#include <string>

class Network {
private:
    bool initialized = false;

public:
    Network() = default;
    ~Network();
    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;
    bool init();
    socket_t createServerSocket(int port);
    socket_t acceptClient(socket_t server_socket);
    int receive(socket_t socket, char* buffer, int size);
    int send(socket_t socket, const char* data, int size);
    void closeSocket(socket_t socket);
    void cleanup();
    bool isInitialized() const;
};