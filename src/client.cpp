#include <iostream>

#include "lib/connection.h"
#include "lib/package.h"

int main() {
    asio::io_context io_context;
    client_connection client{
        []() {
            static size_t pa = 0;
            std::fprintf(stderr, "Successfully written %5zu packages\n", ++pa);
            std::fflush(stderr);
        },
        [](std::string_view error_message) {
            std::fprintf(stderr, "Write error: %.*s\n", static_cast<int>(error_message.size()), error_message.data());
            std::fflush(stderr);
        },
        [](std::variant<message, sign_in, sign_up> &&package) {
            if (package.index() != 0) {
                std::fprintf(stderr, "WTF (sing_in/sign_up)");
                std::fflush(stderr);
            } else {
                message tmp = std::move(std::get<message>(package));
                std::fprintf(stdout, "%s: %s", tmp.name.c_str(), tmp.text.c_str());
                std::fflush(stdout);
            }
        },
        [](std::string_view error_message) {
            std::fprintf(stderr, "Read error: %.*s\n", static_cast<int>(error_message.size()), error_message.data());
            std::fflush(stderr);
        },
        "localhost",
        "56789",
        io_context};
    std::string call;
    message tmp{"kek", "kk"};
    std::cin >> call;
    if (call == "signin") {
        sign_in pack;
        std::cin >> pack.name >> pack.password;
        client.write(pack);
        tmp.name = pack.name;
    }
    if (call == "signup") {
        sign_up pack;
        std::cin >> pack.name >> pack.password;
        client.write(pack);
    }
    tmp.text = call;
    client.write(tmp);
    io_context.run();
}