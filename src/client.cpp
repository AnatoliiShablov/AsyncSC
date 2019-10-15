#include "asio.hpp"

int main(int argc, char *argv[]) {
    asio::io_context io_context;

    asio::ip::tcp::resolver resolver(io_context);
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve("localhost", "56789");

    asio::ip::tcp::socket socket(io_context);
    asio::connect(socket, endpoints);
    std::array<char, 128> buffer{};
    socket.read_some(asio::buffer(buffer));
    return 0;
}