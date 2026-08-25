#include "NetworkFactory.h"

#ifdef _WIN32
#include "WinNetworkFactory.h"
std::unique_ptr<INetworkFactory> makeNetworkFactory() {
    return std::make_unique<WinNetworkFactory>();
}
#else
#include "UnixNetworkFactory.h"
std::unique_ptr<INetworkFactory> makeNetworkFactory() {
    return std::make_unique<UnixNetworkFactory>();
}
#endif