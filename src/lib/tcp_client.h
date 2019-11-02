#ifndef ASYNCSC_TCP_SERVER_H
#define ASYNCSC_TCP_SERVER_H

#include <iostream>
#include <queue>

#include "asio.hpp"
#include "package.h"

class tcp_client {
    asio::io_context io_context;
    asio::ip::tcp::socket server_socket;
    std::queue<package> tasks;
    bool is_writing;
    std::mutex is_writing_mutex;

    void start_read() {
        package pack;
        package::async_read(server_socket, pack, std::bind(&tcp_client::read_handler, this, pack, std::placeholders::_1));
    }

    void start_write() {
        std::lock_guard<std::mutex> lock(is_writing_mutex);
        if (is_writing || tasks.empty()) {
            return;
        }
        package::async_write(server_socket, tasks.front(), std::bind(&tcp_client::write_handler, this, std::placeholders::_1));
    }

    void read_handler(package &pack, asio::error_code const &error) {
        if (!error) {
            std::string message;
            pack >> message;
            std::lock_guard<std::mutex> lock(messages_mutex);
            messages.push(message);
            start_read();
        }
    }

    void write_handler(asio::error_code const &error) {
        if (!error) {
            std::lock_guard<std::mutex> lock(is_writing_mutex);
            is_writing = false;
            tasks.pop();
            start_write();
        }
    }

public:
    std::queue<std::string> messages;
    std::mutex messages_mutex;

    explicit tcp_client(char const *port) : server_socket{io_context} {
        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve("localhost", port);
        asio::connect(server_socket, endpoints);
    }

    void start_work() {
        start_read();
        io_context.run();
    }

    void send(std::string const &message) {
        package pack;
        pack << message;
        tasks.push(pack);
        start_write();
    }
};

#endif  // ASYNCSC_TCP_SERVER_H
