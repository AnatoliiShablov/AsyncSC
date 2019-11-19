#ifndef ASYNCSC_PACKAGE_H
#define ASYNCSC_PACKAGE_H

#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <variant>

#include "asio.hpp"

size_t const BUFFER_SIZE = 16 * 1024;  // 16 KB

enum package_type : uint8_t { MESSAGE, SIGN_IN, SIGN_UP, SPECIAL_SIGNAL, types_amount };

enum class package_state : uint8_t { header_transfering, body_transfering, ready };

uint32_t hash(std::string const &str) noexcept;

struct header {
    uint32_t size;
    package_type type;
};

constexpr size_t header_size_after_serializer() noexcept;

size_t header_serializer(std::byte *output, header const &head);

size_t header_deserializer(std::byte *input, header &head);

struct message {
    std::string name;
    std::string text;
};

uint32_t message_hash(message const &package) noexcept;

size_t message_size_after_serializer(message const &package) noexcept;

size_t message_serializer(std::byte *output, message const &package);

size_t message_deserializer(std::byte *input, message &package);

struct sign_in {
    std::string name;
    std::string password;
};

size_t sign_in_size_after_serializer(sign_in const &package) noexcept;

size_t sign_in_serializer(std::byte *output, sign_in const &package);

size_t sign_in_deserializer(std::byte *input, sign_in &package);

struct sign_up {
    std::string name;
    std::string password;
};

size_t sign_up_size_after_serializer(sign_up const &package) noexcept;

size_t sign_up_serializer(std::byte *output, sign_up const &package);

size_t sign_up_deserializer(std::byte *input, sign_up &package);

struct special_signal {
    enum types : uint8_t {
        NOT_SIGNED,
        ALREADY_SIGNED,
        USER_ALREADY_EXISTS,
        WRONG_PASSWORD,
        WEAK_PASSWORD,
        SIGNED_UP,
        SIGNED_IN,
        SIGN_OUT,
        QUIT,
        special_signals_amount
    };
    types type;

    constexpr explicit special_signal(types type = special_signal::special_signals_amount);
};

constexpr size_t special_signal_size_after_serializer() noexcept;

size_t special_signal_serializer(std::byte *output, special_signal const &package);

size_t special_signal_deserializer(std::byte *input, special_signal &package);

class package_sender {
public:
    package_sender();

    void on_success(std::function<void()> success) noexcept;

    void on_error(std::function<void(std::string_view)> error) noexcept;

    void send_error(std::string_view error);

    int set_package(std::variant<message, sign_in, sign_up, special_signal> const &package);

    [[nodiscard]] package_state get_state() const noexcept;

    [[nodiscard]] auto buffer() noexcept;

    void data_transferred(size_t bytes_transferred);

private:
    [[nodiscard]] size_t left_to_write() const noexcept;

    std::function<void()> success_;
    std::function<void(std::string_view)> error_;

    std::array<std::byte, BUFFER_SIZE> buffer_;
    size_t offset_;
    header header_;

    package_state state_;
};

class package_reciever {
public:
    package_reciever();

    void on_success(std::function<void(std::variant<message, sign_in, sign_up, special_signal> &&)> success) noexcept;

    void on_error(std::function<void(std::string_view)> error) noexcept;

    void send_error(std::string_view error);

    [[nodiscard]] auto buffer() noexcept;

    void data_transferred(size_t bytes_transferred) noexcept;

private:
    [[nodiscard]] size_t left_to_read() const noexcept;

    std::function<void(std::variant<message, sign_in, sign_up, special_signal> &&)> success_;
    std::function<void(std::string_view)> error_;

    std::array<std::byte, BUFFER_SIZE> buffer_;
    size_t offset_;
    header header_;

    package_state state_;
};

#endif  // ASYNCSC_PACKAGE_H
