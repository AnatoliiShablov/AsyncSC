#ifndef ASYNCSC_TCP_SERVER_H
#define ASYNCSC_TCP_SERVER_H

#include <iostream>
#include <unordered_map>
#include <memory>
#include <queue>
#include "asio.hpp"
#include "package.h"

class tcp_server {
    typedef std::shared_ptr<asio::ip::tcp::socket> shared_socket;
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor;
    std::unordered_map<uint32_t, shared_socket> ui_sock;
    std::unordered_map<uint32_t, std::queue<package>> ui_q;
    uint32_t max_id;

    void start_accept() {
        shared_socket new_user(new asio::ip::tcp::socket(io_context));
        acceptor.async_accept(*new_user,
                              std::bind(&tcp_server::accept_handler, this, new_user, std::placeholders::_1));
    }

    void start_read(uint32_t id) {
        package pack;
        package::async_read(ui_sock[id], pack,
                            std::bind(&tcp_server::read_handler, this, id, pack, std::placeholders::_1));
    }

    void start_write(uint32_t id) {
        package::async_write(ui_sock[id], ui_q[id].front(),
                             std::bind(&tcp_server::write_handler, this, id, std::placeholders::_1));
    }

    void accept_handler(shared_socket const &new_user, asio::error_code const &error) {
        if (!error) {
            ui_sock.emplace(++max_id, new_user);
            ui_q.emplace(max_id, std::queue<package>{});
            start_read(max_id);
        }
        start_accept();
    }

    void read_handler(uint32_t id, package &pack, asio::error_code const &error) {
        if (error) {
            ui_sock.erase(id);
            ui_q.erase(id);
            return;
        }

        std::string message;
        pack >> message;
        message = std::to_string(id) + ": " + message;
        pack.clear();
        pack << message;

        for (auto &s : ui_q) {
            s.second.push(pack);
            if (s.second.size() == 1) {
                start_write(s.first);
            }
        }
        start_read(id);
    }

    void write_handler(uint32_t id, asio::error_code const &error) {
        if (error) {
            ui_sock.erase(id);
            ui_q.erase(id);
            return;
        }
        ui_q[id].pop();
        if (!ui_q[id].empty()) {
            start_write(id);
        }
    }

public:
    explicit tcp_server(unsigned short port) :
            io_context{}, acceptor{io_context, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), port}},
            ui_sock{}, ui_q{}, max_id{0} {}

    void start_server() {
        start_accept();
        io_context.run();
    }
};


#endif //ASYNCSC_TCP_SERVER_H
