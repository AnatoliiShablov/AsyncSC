#ifndef ASYNCSC_CONNECTION_H
#define ASYNCSC_CONNECTION_H

#include <queue>

#include "asio.hpp"
#include "package.h"

class client_connection {
public:
    client_connection(std::function<void()> success_write_handler, std::function<void(std::string_view)> error_write_handler,
                      std::function<void(std::variant<message, sign_in, sign_up, special_signal> &&)> success_read_handler,
                      std::function<void(std::string_view)> error_read_handler, asio::string_view host,
                      asio::string_view service, asio::io_context &io_context);

    client_connection(std::function<void()> success_write_handler, std::function<void(std::string_view)> error_write_handler,
                      std::function<void(std::variant<message, sign_in, sign_up, special_signal> &&)> success_read_handler,
                      std::function<void(std::string_view)> error_read_handler, asio::ip::tcp::socket &&socket);

    void write(std::variant<message, sign_in, sign_up, special_signal> const &package);

private:
    void read_loop();

    void write_loop();

    asio::ip::tcp::socket socket_;
    std::unique_ptr<package_sender> sender_;
    std::unique_ptr<package_reciever> reciever_;

    std::queue<std::variant<message, sign_in, sign_up, special_signal>> tasks;
    std::mutex tasks_m;
    std::mutex write_m;
};

class server_connection {
public:
    server_connection(std::function<void(asio::ip::tcp::socket &&)> success_accept_handler,
                      std::function<void(std::string_view)> error_accept_handler, unsigned short port,
                      asio::io_context &io_context);

private:
    void accept_loop();

    asio::ip::tcp::acceptor acceptor_;
    asio::ip::tcp::socket socket_;
    std::function<void(asio::ip::tcp::socket &&)> success_;
    std::function<void(std::string_view)> error_;
};

#endif  // ASYNCSC_CONNECTION_H
