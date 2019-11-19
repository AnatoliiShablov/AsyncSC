cmake_minimum_required(VERSION 3.15)
project(AsyncSC)

set(CMAKE_CXX_STANDARD 17)

execute_process(COMMAND conan install ${CMAKE_CURRENT_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)

find_package(asio REQUIRED)

set(DEBUG ON)

if (${DEBUG})
    add_compile_definitions(debug_fprintf=fprintf)
else ()
    add_compile_definitions(debug_fprintf(...))
endif ()

add_library(package_lib
        src/lib/package.h
        src/lib/package.cpp)
target_link_libraries(package_lib asio::asio)
target_include_directories(package_lib PUBLIC ${ASIO_INCLUDE_DIRS})
# written [✓]      tested [ ]


add_library(connection_lib
        src/lib/connection.h
        src/lib/connection.cpp)
target_link_libraries(connection_lib asio::asio package_lib)
target_include_directories(connection_lib PUBLIC ${ASIO_INCLUDE_DIRS})
# written [✓]      tested [ ]


add_library(server_tcp_lib
        src/lib/tcp_server.cpp
        src/lib/tcp_server.h)
target_link_libraries(server_tcp_lib asio::asio package_lib connection_lib)
target_include_directories(server_tcp_lib PUBLIC ${ASIO_INCLUDE_DIRS})
# written [✓]      tested [ ]

add_executable(AsyncServer src/server.cpp)
target_link_libraries(AsyncServer server_tcp_lib)
target_include_directories(AsyncServer PUBLIC ${ASIO_INCLUDE_DIRS})
# written [✓]      tested [ ]


add_executable(AsyncClient src/client.cpp)
target_link_libraries(AsyncClient asio::asio package_lib connection_lib)
target_include_directories(AsyncClient PUBLIC ${ASIO_INCLUDE_DIRS})
# written [ ]      tested [ ]