cmake_minimum_required(VERSION 3.15)
project(AsyncSC)

set(CMAKE_CXX_STANDARD 17)

execute_process(COMMAND conan install ${CMAKE_CURRENT_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)

find_package(asio REQUIRED)

add_library(server_tcl_lib
        src/lib/tcp_server.cpp
        src/lib/tcp_server.h)

target_link_libraries(server_tcl_lib asio::asio)
target_include_directories(server_tcl_lib PUBLIC ${ASIO_INCLUDE_DIRS})


add_executable(AsyncServer src/server.cpp)
target_link_libraries(AsyncServer server_tcl_lib asio::asio)
target_include_directories(AsyncServer PUBLIC ${ASIO_INCLUDE_DIRS})


add_executable(AsyncClient src/client.cpp)
target_link_libraries(AsyncClient asio::asio)
target_include_directories(AsyncClient PUBLIC ${ASIO_INCLUDE_DIRS})
