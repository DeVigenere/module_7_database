cmake_minimum_required(VERSION 3.10)
project(module_7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(
    include
    include/Calculator
    include/Databaza
    include/Message
    include/Network
    include/sqlite3
    include/nlohmann
)

set(SRC
    src/Network/Network.cpp
    src/Message/Message.cpp
    src/Globals.cpp
    src/Databaza/Databaza.cpp
)

add_executable(receiver src/Receiver/Receiver.cpp ${SRC})
add_executable(calculator_sender src/Calculator_sender/Calculator_sender.cpp src/Calculator/Calculator.cpp ${SRC})

target_link_libraries(receiver ws2_32 sqlite3)
target_link_libraries(calculator_sender ws2_32 sqlite3)
target_link_directories(receiver PRIVATE bin/sqlite3)
target_link_directories(calculator_sender PRIVATE bin/sqlite3)