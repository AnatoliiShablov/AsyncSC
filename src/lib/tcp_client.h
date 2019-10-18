#ifndef ASYNCSC_TCP_SERVER_H
#define ASYNCSC_TCP_SERVER_H

#include <iostream>
#include <deque>
#include <vector>
#include <unordered_map>
#include <memory>
#include "asio.hpp"

class tcp_client {
    typedef std::shared_ptr<asio::ip::tcp::socket> shared_socket;
    struct socket_with_data {
        shared_socket socket;
        std::deque<char> data;
        std::vector<char> buf;
        uint32_t text_size;
    };
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor;
    std::unordered_map<shared_socket, uint32_t> sock_ui;
    std::unordered_map<uint32_t, socket_with_data> ui_sock;
    uint32_t max_id;

    void start_accept() {
        shared_socket new_user(new asio::ip::tcp::socket(io_context));
        acceptor.async_accept(*new_user, std::bind(&tcp_server::accept_handler, this, new_user, std::placeholders::_1));
    }

    static void write_handler(asio::error_code const &, size_t) {

    }

    void read_handler(uint32_t id, asio::error_code const &error, size_t bytes_transferred) {
        auto which_socket = ui_sock[id];
        if (error) {
            sock_ui.erase(which_socket.socket);
            ui_sock.erase(id);
            return;
        }
        for (size_t i = 0; i < bytes_transferred; ++i) {
            which_socket.data.push_back(which_socket.buf[i]);
        }
        if (which_socket.text_size == 0 && which_socket.data.size() >= 4) {
            which_socket.text_size |= static_cast<uint32_t>(which_socket.data.front());
            which_socket.data.pop_front();
            which_socket.text_size <<= 8u;

            which_socket.text_size |= static_cast<uint32_t>(which_socket.data.front());
            which_socket.data.pop_front();
            which_socket.text_size <<= 8u;


            which_socket.text_size |= static_cast<uint32_t>(which_socket.data.front());
            which_socket.data.pop_front();
            which_socket.text_size <<= 8u;


            which_socket.text_size |= static_cast<uint32_t>(which_socket.data.front());
            which_socket.data.pop_front();
        }
        if (which_socket.text_size > 0 && which_socket.data.size() >= which_socket.text_size) {
            std::string message = std::to_string(id) + " :: ";
            for (size_t i = 0; i < which_socket.text_size; ++i) {
                message.push_back(which_socket.data.front());
                which_socket.data.pop_front();
            }
            std::vector<shared_socket> recievers(sock_ui.size());
            std::transform(sock_ui.begin(), sock_ui.end(), recievers.begin(), [](auto pair) { return pair.first; });
            std::for_each(recievers.begin(), recievers.end(),
                          [message](auto sock) {
                              asio::async_write(*sock, asio::buffer(message), &tcp_server::write_handler);
                          });
        }
        which_socket.socket->async_read_some(asio::buffer(which_socket.buf),
                                             std::bind(&tcp_server::read_handler, this, max_id,
                                                       std::placeholders::_1, std::placeholders::_2));
    }

    void accept_handler(shared_socket const &new_user, asio::error_code const &error) {
        if (!error) {
            sock_ui.emplace(new_user, ++max_id);
            ui_sock.emplace(max_id,
                            socket_with_data{new_user, std::deque<char>(), std::vector<char>(128), 0});
            new_user->async_read_some(asio::buffer(ui_sock[max_id].buf),
                                      std::bind(&tcp_server::read_handler, this, max_id,
                                                std::placeholders::_1, std::placeholders::_2));
        }
        start_accept();
    }

public:
    tcp_client(unsigned short port){
        asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints =
                resolver.resolve(argv[1], "daytime");

        tcp::socket socket(io_context);
        asio::connect(socket, endpoints);
    }

    void start_work() {
        start_accept();
        io_context.run();
    }
};


#endif //ASYNCSC_TCP_SERVER_H
