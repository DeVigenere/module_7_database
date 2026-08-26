cmake_minimum_required(VERSION 3.31.6)

set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/vcpkg")
set(VCPKG_EXECUTABLE "${VCPKG_ROOT}/vcpkg")

if(WIN32)
    set(VCPKG_CMD "${VCPKG_EXECUTABLE}.exe")
else()
    set(VCPKG_CMD "${VCPKG_EXECUTABLE}")
endif()

if(NOT EXISTS "${VCPKG_ROOT}")
    execute_process(
        COMMAND git clone --depth=1 https://github.com/Microsoft/vcpkg.git "${VCPKG_ROOT}"
        RESULT_VARIABLE GIT_RESULT
        OUTPUT_VARIABLE GIT_OUTPUT
        ERROR_VARIABLE GIT_ERROR
    )
endif()

if(NOT EXISTS "${VCPKG_CMD}")
    if(WIN32)
        execute_process(
            COMMAND "${VCPKG_ROOT}/bootstrap-vcpkg.bat" -disableMetrics
            WORKING_DIRECTORY "${VCPKG_ROOT}"
            RESULT_VARIABLE BOOTSTRAP_RESULT
            OUTPUT_VARIABLE BOOTSTRAP_OUTPUT
            ERROR_VARIABLE BOOTSTRAP_ERROR
        )
    else()
        execute_process(
            COMMAND "${VCPKG_ROOT}/bootstrap-vcpkg.sh" -disableMetrics
            WORKING_DIRECTORY "${VCPKG_ROOT}"
            RESULT_VARIABLE BOOTSTRAP_RESULT
            OUTPUT_VARIABLE BOOTSTRAP_OUTPUT
            ERROR_VARIABLE BOOTSTRAP_ERROR
        )
    endif()
endif()

execute_process(
    COMMAND "${VCPKG_CMD}" install --x-manifest-root="${CMAKE_SOURCE_DIR}" --x-install-root="${CMAKE_SOURCE_DIR}/vcpkg_installed"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE VCPKG_RESULT
    OUTPUT_VARIABLE VCPKG_OUTPUT
    ERROR_VARIABLE VCPKG_ERROR
)

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)

project(module_7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

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
    src/Network/NetworkFactory.cpp
    src/Message/Message.cpp
    src/Globals.cpp
    src/Databaza/SqliteDatabase.cpp
)

if(WIN32)
    set(PLATFORM_SRC
        src/Network/WinNetworkFactory.cpp
        src/Network/WinConnection.cpp
	src/Network/WinListener.cpp
    )
else()
    set(PLATFORM_SRC
        src/Network/UnixNetworkFactory.cpp
        src/Network/UnixConnection.cpp
	src/Network/UnixListener.cpp
    )
endif()

add_executable(receiver src/Receiver/Receiver.cpp ${COMMON_SRC} ${PLATFORM_SRC})
add_executable(calculator_sender src/Calculator_sender/Calculator_sender.cpp src/Calculator/Calculator.cpp ${COMMON_SRC} ${PLATFORM_SRC})

target_link_libraries(receiver ${SQLite3_LIBRARIES} nlohmann_json::nlohmann_json)
target_link_libraries(calculator_sender ${SQLite3_LIBRARIES} nlohmann_json::nlohmann_json)

if(WIN32)
    target_link_libraries(receiver ws2_32)
    target_link_libraries(calculator_sender ws2_32)
endif()

if(UNIX)
    target_link_libraries(receiver pthread)
    target_link_libraries(calculator_sender pthread)
endif()