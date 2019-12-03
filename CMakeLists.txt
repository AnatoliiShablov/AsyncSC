cmake_minimum_required(VERSION 3.5)

project(FireFly)

execute_process(COMMAND conan install ${CMAKE_CURRENT_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(asio REQUIRED)

set(DEBUG OFF)

if (${DEBUG})
    add_compile_definitions(DEBUG_OUTPUT)
endif ()

set (LIBRARIES_SERVER asio::asio Qt5::Core)

add_library(package_lib
        libs/include/package.h
        libs/src/package.cpp)
target_link_libraries(package_lib ${LIBRARIES_SERVER})
target_include_directories(package_lib PUBLIC ${ASIO_INCLUDE_DIRS})
# written [ ]      tested [ ]

add_library(connection_lib
        libs/include/connection.h
        libs/src/connection.cpp)
target_link_libraries(connection_lib ${LIBRARIES_SERVER} package_lib)
target_include_directories(connection_lib PUBLIC ${ASIO_INCLUDE_DIRS})
# written [✓]      tested [✓]

add_library(server_tcp_lib 
        libs/include/tcp_server.cpp
        libs/src/tcp_server.h)
target_link_libraries(server_tcp_lib ${LIBRARIES_SERVER} package_lib connection_lib)
target_include_directories(server_tcp_lib PUBLIC ${ASIO_INCLUDE_DIRS})
# written [✓]      tested [✓]

add_executable(AsyncServer src/server.cpp)
target_link_libraries(AsyncServer ${LIBRARIES_SERVER} server_tcp_lib connection_lib package_lib)
target_include_directories(AsyncServer PUBLIC ${ASIO_INCLUDE_DIRS})
# written [✓]      tested [✓]


set (SOURCES src/client.cpp src/mainwindow.cpp)

set (HEADERS src/mainwindow.h )

set (LIBRARIES Qt5::Widgets asio::asio connection_lib package_lib)

set (RESOURCES)

set (UI_HEADERS ui_forms/ui_mainwindow.h)

set (UI_FORMS ui_forms/mainwindow.ui)

add_executable(FireFly ${HEADERS} ${SOURCES} ${UI_FORMS} ${UI_HEADERS} ${RESOURCES})
target_link_libraries(FireFly ${LIBRARIES})
# written [ ]      tested [ ]
