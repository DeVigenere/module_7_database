cmake_minimum_required(VERSION 3.10)
project(module_7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(SQLite3 REQUIRED)

include_directories(
    include
    include/Calculator
    include/Databaza
    include/Message
    include/Network
    include/nlohmann
    ${SQLite3_INCLUDE_DIRS}
)

set(COMMON_SRC
    src/Network/Network.cpp
    src/Message/Message.cpp
    src/Globals.cpp
    src/Databaza/SqliteDatabase.cpp
)

add_executable(receiver src/Receiver/Receiver.cpp ${COMMON_SRC})
add_executable(calculator_sender src/Calculator_sender/Calculator_sender.cpp src/Calculator/Calculator.cpp ${COMMON_SRC})

target_link_libraries(receiver ${SQLite3_LIBRARIES})
target_link_libraries(calculator_sender ${SQLite3_LIBRARIES})

if(WIN32)
    target_link_libraries(receiver ws2_32)
    target_link_libraries(calculator_sender ws2_32)
endif()