cmake_minimum_required(VERSION 3.10)
project(DATABAZAAAAAAA)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SQLITE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SQLite)

include_directories(${SQLITE_DIR}/include)
link_directories(${SQLITE_DIR}/lib)

add_executable(receiver receiver.cpp)
target_link_libraries(receiver 
    ws2_32 
    sqlite3
)

add_executable(calculator_sender 
    calculator_sender.cpp 
    Calculator.cpp
)
target_link_libraries(calculator_sender 
    ws2_32
)

add_custom_command(TARGET receiver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SQLITE_DIR}/bin/sqlite3.dll
        $<TARGET_FILE_DIR:receiver>
)

add_custom_command(TARGET calculator_sender POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SQLITE_DIR}/bin/sqlite3.dll
        $<TARGET_FILE_DIR:calculator_sender>
)