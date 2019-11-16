#include "lib/tcp_server.h"

int main() {
    asio::io_context io_context;
    tcp_server server(56789, io_context);
    io_context.run();
    return 0;
}