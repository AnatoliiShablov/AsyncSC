#include <iostream>
#include "asio.hpp"

int main(int argc, char *argv[]) {
    try {
        asio::io_context io_context;

        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve("localhost", "56789");

        asio::ip::tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        for (;;) {
            std::array<char, 128> buf{};
            asio::error_code error;

            size_t len = socket.read_some(asio::buffer(buf), error);

            if (error == asio::error::eof)
                break;
            else if (error)
                throw asio::system_error(error);

            std::cout.write(buf.data(), len);
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}