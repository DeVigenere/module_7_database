#pragma once
#include "Network/INetworkFactory.h"
#include <memory>

std::unique_ptr<INetworkFactory> makeNetworkFactory();