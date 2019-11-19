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
                      asio::string_view service, asio::io_context &io_context)
        : socket_{io_context}, sender_{new package_sender{}}, reciever_{new package_reciever{}} {
        reciever_->on_error(std::move(error_read_handler));
        reciever_->on_success(std::move(success_read_handler));
        sender_->on_success(std::move(success_write_handler));
        sender_->on_error(std::move(error_write_handler));

        asio::ip::tcp::resolver resolver(io_context);
        asio::connect(socket_, asio::ip::tcp::resolver(io_context).resolve(host, service));
        read_loop();
    }

    client_connection(std::function<void()> success_write_handler, std::function<void(std::string_view)> error_write_handler,
                      std::function<void(std::variant<message, sign_in, sign_up, special_signal> &&)> success_read_handler,
                      std::function<void(std::string_view)> error_read_handler, asio::ip::tcp::socket &&socket)
        : socket_{std::move(socket)}, sender_{new package_sender{}}, reciever_{new package_reciever{}} {
        reciever_->on_error(std::move(error_read_handler));
        reciever_->on_success(std::move(success_read_handler));
        sender_->on_success(std::move(success_write_handler));
        sender_->on_error(std::move(error_write_handler));

        read_loop();
    }

    void write(std::variant<message, sign_in, sign_up, special_signal> const &package) {
        std::lock_guard<std::mutex> lock_queue(tasks_m);
        if (write_m.try_lock()) {
            sender_->set_package(package);
            write_loop();
        } else {
            tasks.push(package);
        }
    }

private:
    void read_loop() {
        socket_.async_read_some(reciever_->buffer(), [this](asio::error_code const &error, size_t bytes_recieved) {
            if (error) {
                reciever_->send_error(error.message());
                return;
            }
            reciever_->data_transferred(bytes_recieved);
            read_loop();
        });
    }

    void write_loop() {
        socket_.async_write_some(sender_->buffer(), [this](asio::error_code const &error, size_t bytes_sent) {
            if (error) {
                sender_->send_error(error.message());
                write_m.unlock();
            } else {
                sender_->data_transferred(bytes_sent);
                std::lock_guard<std::mutex> lock_queue(tasks_m);
                if (sender_->get_state() == package_state::ready) {
                    if (!tasks.empty() && sender_->set_package(tasks.front()) == 0) {
                        tasks.pop();
                        write_loop();
                    } else {
                        write_m.unlock();
                    }
                } else {
                    write_loop();
                }
            }
        });
    }

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
                      asio::io_context &io_context)
        : acceptor_{io_context, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), port}}
        , socket_{io_context}
        , success_{std::move(success_accept_handler)}
        , error_{std::move(error_accept_handler)} {
        accept_loop();
    }

private:
    void accept_loop() {
        acceptor_.async_accept(socket_, [this](asio::error_code const &error) {
            debug_fprintf(stdout, "%s:%hu", socket_.remote_endpoint().address().to_string().c_str(),
                          socket_.remote_endpoint().port());
            if (error) {
                error_(error.message());
                decltype(socket_)(std::move(socket_));
            } else {
                success_(std::move(socket_));
            }
            accept_loop();
        });
    }

    asio::ip::tcp::acceptor acceptor_;
    asio::ip::tcp::socket socket_;
    std::function<void(asio::ip::tcp::socket &&)> success_;
    std::function<void(std::string_view)> error_;
};

#endif  // ASYNCSC_CONNECTION_H
