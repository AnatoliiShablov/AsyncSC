#ifndef ASYNCSC_TCP_SERVER_H
#define ASYNCSC_TCP_SERVER_H

#include <cstdint>
#include <cstdio>
#include <unordered_map>

#include "asio.hpp"
#include "connection.h"
#include "package.h"

class tcp_server {
public:
    explicit tcp_server(unsigned short port, asio::io_context &context)
        : max_id{}
        , server{[this](asio::ip::tcp::socket &&new_user) {
                     std::fprintf(stdout, "New connection\n");
                     std::fflush(stdout);
                     counter[++max_id] = std::make_pair(0, 0);
                     std::cout << "USERS ID=" << max_id << std::endl;
                     users.emplace(max_id,
                                   new client_connection{
                                       std::bind(&tcp_server::success_write_handler, this, max_id),
                                       std::bind(&tcp_server::error_write_handler, this, max_id, std::placeholders::_1),
                                       std::bind(&tcp_server::success_read_handler, this, max_id, std::placeholders::_1),
                                       std::bind(&tcp_server::error_read_handler, this, max_id, std::placeholders::_1),
                                       std::forward<asio::ip::tcp::socket>(new_user)});
                 },
                 [this](std::string_view error_message) {
                     std::fprintf(stderr, "%.*s\n", static_cast<int>(error_message.size()), error_message.data());
                     std::fflush(stderr);
                 },
                 port, context} {}

private:
    void success_write_handler(size_t id) {
        std::fprintf(stdout, "successfully written (id : %5zu) : %5zu packages\n", id, ++counter[id].first);
        std::cout << "USERS ID(WRITE)=" << id << std::endl;
        std::fflush(stdout);
    }

    void error_write_handler(size_t id, std::string_view error_message) {
        std::fprintf(stderr, "Write error on id : %5zu\n%.*s\n", id, static_cast<int>(error_message.size()),
                     error_message.data());
        std::fflush(stderr);
        counter.erase(id);
        connected_users.erase(id);
        users.erase(id);
    }

    void success_read_handler(size_t id, std::variant<message, sign_in, sign_up> &&package) {
        std::fprintf(stdout, "successfully read (id : %5zu) : %5zu packages\n", id, ++counter[id].second);
        std::fflush(stdout);
        std::cout << "USERS ID(READ)=" << id << std::endl;
        if (!connected_users.count(id) && package.index() == 0) {
            std::fprintf(stderr, "User on id : %5zu isn't signed\n", id);
            std::fflush(stderr);
            message signal{"admin", "You're not signed"};
            users[id]->write(signal);
            return;
        }
        if (connected_users.count(id) && package.index() != 0) {
            std::fprintf(stderr, "User on id : %5zu already signed\n", id);
            std::fflush(stderr);
            message signal{"admin", "You're already signed"};
            users[id]->write(signal);
            return;
        }
        if (package.index() == 0) {
            for (auto &connection : users) {
                if (connected_users.count(connection.first) && connection.first != id) {
                    connection.second->write(package);
                }
            }
        }
        if (package.index() == 1) {
            sign_in tmp = std::move(std::get<sign_in>(package));
            if (users_base[tmp.name] == tmp.password) {
                connected_users[id] = tmp.name;
            } else {
                std::fprintf(stderr, "User on id : %5zu wrong password\n", id);
                std::fflush(stderr);
                message signal{"admin", "Wrong password"};
                users[id]->write(signal);
            }
        }
        if (package.index() == 2) {
            sign_up tmp = std::move(std::get<sign_up>(package));
            if (users_base.count(tmp.name)) {
                std::fprintf(stderr, "User on id : %5zu such user already exists\n", id);
                std::fflush(stderr);
                message signal{"admin", "Such user already exists"};
                users[id]->write(signal);
            } else {
                if (tmp.password.length() < 5) {
                    std::fprintf(stderr, "User on id : %5zu Password is too simple\n", id);
                    std::fflush(stderr);
                    message signal{"admin", "Password should be 5 or more symbols"};
                    users[id]->write(signal);
                } else {
                    users_base[tmp.name] = tmp.password;
                }
            }
        }
    }

    void error_read_handler(size_t id, std::string_view error_message) {
        std::fprintf(stderr, "Read error on id : %5zu\n%.*s\n", id, static_cast<int>(error_message.size()),
                     error_message.data());
        std::fflush(stderr);
        users.erase(id);
        counter.erase(id);
        connected_users.erase(id);
    }

    std::unordered_map<uint32_t, std::unique_ptr<client_connection>> users;
    std::unordered_map<uint32_t, std::string> connected_users;
    std::unordered_map<std::string, std::string> users_base;

    std::unordered_map<uint32_t, std::pair<size_t, size_t>> counter;
    uint32_t max_id;

    server_connection server;
};

#endif  // ASYNCSC_TCP_SERVER_H
