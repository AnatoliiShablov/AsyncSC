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
    tcp_server(unsigned short port, asio::io_context &context);

private:
    void success_write_handler(size_t id);

    void error_write_handler(size_t id, std::string_view error_message);

    void success_read_handler(size_t id, std::variant<message, sign_in, sign_up, special_signal> &&package);

    void error_read_handler(size_t id, std::string_view error_message);

    std::unordered_map<uint32_t, std::unique_ptr<client_connection>> users;
    std::unordered_map<uint32_t, std::string> connected_users;
    std::unordered_map<std::string, std::string> users_base;

    std::unordered_map<uint32_t, std::pair<size_t, size_t>> counter;
    uint32_t max_id;

    server_connection server;
};

#endif  // ASYNCSC_TCP_SERVER_H
