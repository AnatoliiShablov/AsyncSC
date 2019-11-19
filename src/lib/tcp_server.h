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
                     debug_fprintf(stdout, "New connection with id %u\n", ++max_id);
                     counter[max_id] = std::make_pair(0, 0);
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
                 },
                 port, context} {}

private:
    void success_write_handler(size_t id) {
        debug_fprintf(stdout, "Successfully written (id : %5zu) : %5zu packages\n", id, ++counter[id].first);
    }

    void error_write_handler(size_t id, std::string_view error_message) {
        std::fprintf(stderr, "Write error : %5zu\n%.*s\n", id, static_cast<int>(error_message.size()), error_message.data());
        counter.erase(id);
        connected_users.erase(id);
        users.erase(id);
    }

    void success_read_handler(size_t id, std::variant<message, sign_in, sign_up, special_signal> &&package) {
        debug_fprintf(stdout, "Successfully read (id : %5zu) : %5zu packages\n", id, ++counter[id].second);
        switch (package.index()) {
        case 0: {
            if (connected_users.count(id)) {
                for (auto &connection : users) {
                    if (connected_users.count(connection.first) && connection.first != id) {
                        connection.second->write(package);
                    }
                }
            } else {
                std::fprintf(stderr, "User isn't signed (id : %5zu)\n", id);
                users[id]->write(special_signal::NOT_SIGNED);
            }
            return;
        }
        case 1: {
            if (!connected_users.count(id)) {
                sign_in tmp = std::move(std::get<sign_in>(package));
                if (users_base.count(tmp.name) && users_base[tmp.name] == tmp.password) {
                    connected_users[id] = tmp.name;
                } else {
                    std::fprintf(stderr, "Wrong password(id : %5zu)\n", id);
                    users[id]->write(special_signal::WRONG_PASSWORD);
                }
            } else {
                std::fprintf(stderr, "Already signed (id : %5zu)\n", id);
                users[id]->write(special_signal::ALREADY_SIGNED);
            }
            return;
        }
        case 2: {
            if (!connected_users.count(id)) {
                sign_up tmp = std::move(std::get<sign_up>(package));
                if (users_base.count(tmp.name)) {
                    debug_fprintf(stdout, "Such user already exists (id : %5zu)\n", id);
                    users[id]->write(special_signal::USER_ALREADY_EXISTS);
                } else {
                    if (tmp.password.length() < 5) {
                        debug_fprintf(stdout, "Password is simple (id : %5zu)\n", id);
                        users[id]->write(special_signal::WEAK_PASSWORD);
                    } else {
                        users_base[tmp.name] = tmp.password;
                        users[id]->write(special_signal::SIGNED_UP);
                        debug_fprintf(stdout, "Successfully registered (id : %5zu) with name : %s\n", id, tmp.name.c_str());
                    }
                }
            } else {
                std::fprintf(stderr, "Already signed (id : %5zu)\n", id);
                users[id]->write(special_signal::ALREADY_SIGNED);
            }
            return;
        }
        case 3: {
            switch (std::get<special_signal>(package).type) {
            case special_signal::QUIT: {
                debug_fprintf(stdout, "Log out (id : %5zu)\n", id);
                counter.erase(id);
                connected_users.erase(id);
                users.erase(id);
                break;
            }
            case special_signal::SIGN_OUT: {
                debug_fprintf(stdout, "Log out (id : %5zu)\n", id);
                connected_users.erase(id);
                break;
            }
            default: {
                std::fprintf(stderr, "Strange siganl (id : %5zu)\n", id);
            }
            }
            return;
        }
        }
        std::fprintf(stderr, "Something wrong with variant\n");
    }

    void error_read_handler(size_t id, std::string_view error_message) {
        std::fprintf(stderr, "Read error on id : %5zu\n%.*s\n", id, static_cast<int>(error_message.size()),
                     error_message.data());
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
