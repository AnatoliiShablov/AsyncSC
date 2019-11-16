#ifndef ASYNCSC_PACKAGE_H
#define ASYNCSC_PACKAGE_H

#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <variant>

#include "asio.hpp"

size_t const BUFFER_SIZE = 16 * 1024;  // 16 KB

enum package_type : uint8_t { MESSAGE, SIGN_IN, SIGN_UP };
enum class package_state : uint8_t { header_transfering, body_transfering, ready };

uint32_t hash(std::string const &str) {
    uint32_t hash_ = 0;
    uint32_t mult = 1;
    uint32_t alphabet_size = 257;
    for (char c : str) {
        hash_ += static_cast<uint32_t>(c) * mult;
        mult *= alphabet_size;
    }
    return hash_;
}

struct header {
    uint32_t size;
    package_type type;
};

size_t header_serializer(std::byte *output, header const &head) {
    output[0] = static_cast<std::byte>((head.size & UINT32_C(0xff000000)) >> UINT32_C(24));
    output[1] = static_cast<std::byte>((head.size & UINT32_C(0x00ff0000)) >> UINT32_C(16));
    output[2] = static_cast<std::byte>((head.size & UINT32_C(0x0000ff00)) >> UINT32_C(8));
    output[3] = static_cast<std::byte>((head.size & UINT32_C(0x000000ff)) >> UINT32_C(0));

    output[4] = static_cast<std::byte>(head.type);

    return sizeof(header);
}

size_t header_deserializer(std::byte *input, header &head) {
    head.size = static_cast<uint32_t>(input[0]) << UINT32_C(24) | static_cast<uint32_t>(input[1]) << UINT32_C(16) |
                static_cast<uint32_t>(input[2]) << UINT32_C(8) | static_cast<uint32_t>(input[3]) << UINT32_C(0);

    auto type = static_cast<uint8_t>(input[4]);
    if (type > 2) {
        return 0;
    }
    head.type = static_cast<package_type>(type);

    return sizeof(header);
}

struct message {
    std::string name;
    // time_ext time;
    std::string text;
};

uint32_t message_hash(message const &package) {
    return hash(package.name) ^ hash(package.text);
}

size_t message_serializer(std::byte *output, message const &package) {
    uint32_t name_length = package.name.length();
    uint32_t text_length = package.text.length();
    uint32_t hash = message_hash(package);

    output[0] = static_cast<std::byte>((name_length & UINT32_C(0xff000000)) >> UINT32_C(24));
    output[1] = static_cast<std::byte>((name_length & UINT32_C(0x00ff0000)) >> UINT32_C(16));
    output[2] = static_cast<std::byte>((name_length & UINT32_C(0x0000ff00)) >> UINT32_C(8));
    output[3] = static_cast<std::byte>((name_length & UINT32_C(0x000000ff)) >> UINT32_C(0));

    output[4] = static_cast<std::byte>((text_length & UINT32_C(0xff000000)) >> UINT32_C(24));
    output[5] = static_cast<std::byte>((text_length & UINT32_C(0x00ff0000)) >> UINT32_C(16));
    output[6] = static_cast<std::byte>((text_length & UINT32_C(0x0000ff00)) >> UINT32_C(8));
    output[7] = static_cast<std::byte>((text_length & UINT32_C(0x000000ff)) >> UINT32_C(0));

    output[8] = static_cast<std::byte>((hash & UINT32_C(0xff000000)) >> UINT32_C(24));
    output[9] = static_cast<std::byte>((hash & UINT32_C(0x00ff0000)) >> UINT32_C(16));
    output[10] = static_cast<std::byte>((hash & UINT32_C(0x0000ff00)) >> UINT32_C(8));
    output[11] = static_cast<std::byte>((hash & UINT32_C(0x000000ff)) >> UINT32_C(0));

    size_t offset = 12;
    for (char c : package.name) {
        output[offset++] = static_cast<std::byte>(c);
    }

    for (char c : package.text) {
        output[offset++] = static_cast<std::byte>(c);
    }

    return offset;
}

size_t message_deserializer(std::byte *input, message &package) {
    uint32_t name_length = static_cast<uint32_t>(input[0]) << UINT32_C(24) | static_cast<uint32_t>(input[1]) << UINT32_C(16) |
                           static_cast<uint32_t>(input[2]) << UINT32_C(8) | static_cast<uint32_t>(input[3]) << UINT32_C(0);
    uint32_t text_length = static_cast<uint32_t>(input[4]) << UINT32_C(24) | static_cast<uint32_t>(input[5]) << UINT32_C(16) |
                           static_cast<uint32_t>(input[6]) << UINT32_C(8) | static_cast<uint32_t>(input[7]) << UINT32_C(0);
    uint32_t hash = static_cast<uint32_t>(input[8]) << UINT32_C(24) | static_cast<uint32_t>(input[9]) << UINT32_C(16) |
                    static_cast<uint32_t>(input[10]) << UINT32_C(8) | static_cast<uint32_t>(input[11]) << UINT32_C(0);

    package.name.resize(name_length);
    package.text.resize(text_length);

    size_t offset = 12;
    for (size_t i = 0; i < name_length; ++i) {
        package.name[i] = static_cast<char>(input[offset++]);
    }

    for (size_t i = 0; i < text_length; ++i) {
        package.text[i] = static_cast<char>(input[offset++]);
    }

    return hash == message_hash(package) ? offset : 0;
}

struct sign_in {
    std::string name;
    std::string password;
};

size_t sign_in_serializer(std::byte *output, sign_in const &package) {
    uint32_t name_length = package.name.length();
    uint32_t password_length = package.password.length();

    output[0] = static_cast<std::byte>((name_length & UINT32_C(0xff000000)) >> UINT32_C(24));
    output[1] = static_cast<std::byte>((name_length & UINT32_C(0x00ff0000)) >> UINT32_C(16));
    output[2] = static_cast<std::byte>((name_length & UINT32_C(0x0000ff00)) >> UINT32_C(8));
    output[3] = static_cast<std::byte>((name_length & UINT32_C(0x000000ff)) >> UINT32_C(0));

    output[4] = static_cast<std::byte>((password_length & UINT32_C(0xff000000)) >> UINT32_C(24));
    output[5] = static_cast<std::byte>((password_length & UINT32_C(0x00ff0000)) >> UINT32_C(16));
    output[6] = static_cast<std::byte>((password_length & UINT32_C(0x0000ff00)) >> UINT32_C(8));
    output[7] = static_cast<std::byte>((password_length & UINT32_C(0x000000ff)) >> UINT32_C(0));

    size_t offset = 8;
    for (char c : package.name) {
        output[offset++] = static_cast<std::byte>(c);
    }

    for (char c : package.password) {
        output[offset++] = static_cast<std::byte>(c);
    }

    return offset;
}

size_t sign_in_deserializer(std::byte *input, sign_in &package) {
    uint32_t name_length = static_cast<uint32_t>(input[0]) << UINT32_C(24) | static_cast<uint32_t>(input[1]) << UINT32_C(16) |
                           static_cast<uint32_t>(input[2]) << UINT32_C(8) | static_cast<uint32_t>(input[3]) << UINT32_C(0);
    uint32_t password_length = static_cast<uint32_t>(input[4]) << UINT32_C(24) |
                               static_cast<uint32_t>(input[5]) << UINT32_C(16) |
                               static_cast<uint32_t>(input[6]) << UINT32_C(8) | static_cast<uint32_t>(input[7]) << UINT32_C(0);

    package.name.resize(name_length);
    package.password.resize(password_length);

    size_t offset = 8;
    for (size_t i = 0; i < name_length; ++i) {
        package.name[i] = static_cast<char>(input[offset++]);
    }

    for (size_t i = 0; i < password_length; ++i) {
        package.password[i] = static_cast<char>(input[offset++]);
    }

    return offset;
}

struct sign_up {
    std::string name;
    std::string password;
};

size_t sign_up_serializer(std::byte *output, sign_up const &package) {
    uint32_t name_length = package.name.length();
    uint32_t password_length = package.password.length();

    output[0] = static_cast<std::byte>((name_length & UINT32_C(0xff000000)) >> UINT32_C(24));
    output[1] = static_cast<std::byte>((name_length & UINT32_C(0x00ff0000)) >> UINT32_C(16));
    output[2] = static_cast<std::byte>((name_length & UINT32_C(0x0000ff00)) >> UINT32_C(8));
    output[3] = static_cast<std::byte>((name_length & UINT32_C(0x000000ff)) >> UINT32_C(0));

    output[4] = static_cast<std::byte>((password_length & UINT32_C(0xff000000)) >> UINT32_C(24));
    output[5] = static_cast<std::byte>((password_length & UINT32_C(0x00ff0000)) >> UINT32_C(16));
    output[6] = static_cast<std::byte>((password_length & UINT32_C(0x0000ff00)) >> UINT32_C(8));
    output[7] = static_cast<std::byte>((password_length & UINT32_C(0x000000ff)) >> UINT32_C(0));

    size_t offset = 8;
    for (char c : package.name) {
        output[offset++] = static_cast<std::byte>(c);
    }

    for (char c : package.password) {
        output[offset++] = static_cast<std::byte>(c);
    }

    return offset;
}

size_t sign_up_deserializer(std::byte *input, sign_up &package) {
    uint32_t name_length = static_cast<uint32_t>(input[0]) << UINT32_C(24) | static_cast<uint32_t>(input[1]) << UINT32_C(16) |
                           static_cast<uint32_t>(input[2]) << UINT32_C(8) | static_cast<uint32_t>(input[3]) << UINT32_C(0);
    uint32_t password_length = static_cast<uint32_t>(input[4]) << UINT32_C(24) |
                               static_cast<uint32_t>(input[5]) << UINT32_C(16) |
                               static_cast<uint32_t>(input[6]) << UINT32_C(8) | static_cast<uint32_t>(input[7]) << UINT32_C(0);

    package.name.resize(name_length);
    package.password.resize(password_length);

    size_t offset = 8;
    for (size_t i = 0; i < name_length; ++i) {
        package.name[i] = static_cast<char>(input[offset++]);
    }

    for (size_t i = 0; i < password_length; ++i) {
        package.password[i] = static_cast<char>(input[offset++]);
    }

    return offset;
}

class package_sender {
public:
    package_sender() : success_{}, error_{}, buffer_{}, offset_{0}, header_{}, state_{package_state::ready} {}

    void on_success(std::function<void()> success) { success_ = std::move(success); }

    void on_error(std::function<void(std::string_view)> error) { error_ = std::move(error); }

    void send_error(std::string_view error) { error_(error); }

    void send_success() { success_(); }

    int set_package(std::variant<message, sign_in, sign_up> const &package) {
        if (state_ != package_state::ready) {
            error_("Sender is busy");
            return -1;
        }
        switch (package.index()) {
        case 0: {
            header_.type = package_type::MESSAGE;
            header_.size =
                2 * sizeof(uint32_t) + std::get<message>(package).name.length() + std::get<message>(package).text.length();
            break;
        }
        case 1: {
            header_.type = package_type::SIGN_IN;
            header_.size =
                2 * sizeof(uint32_t) + std::get<sign_in>(package).name.length() + std::get<sign_in>(package).password.length();
            break;
        }
        case 2: {
            header_.type = package_type::SIGN_UP;
            header_.size =
                2 * sizeof(uint32_t) + std::get<sign_up>(package).name.length() + std::get<sign_up>(package).password.length();
            break;
        }
        }
        if (header_.size > BUFFER_SIZE) {
            error_("Package is too big");
            return -2;
        }
        offset_ = header_serializer(buffer_.data(), header_);
        switch (package.index()) {
        case 0: {
            message_serializer(buffer_.data() + offset_, std::get<message>(package));
            break;
        }
        case 1: {
            sign_in_serializer(buffer_.data() + offset_, std::get<sign_in>(package));
            break;
        }
        case 2: {
            sign_up_serializer(buffer_.data() + offset_, std::get<sign_up>(package));
            break;
        }
        }
        offset_ = 0;
        state_ = package_state::header_transfering;
        return 0;
    };

    package_state get_state() { return state_; }

    [[nodiscard]] auto buffer() {
        if (left_to_write() != 0)
            std::cout << "write " << left_to_write() << std::endl;
        return asio::buffer(buffer_.data() + offset_, left_to_write());
    }

    void data_transferred(size_t bytes_transferred) {
        if (bytes_transferred != 0)
            std::cout << "write-transfered " << bytes_transferred << std::endl;
        offset_ += bytes_transferred;
        if (state_ == package_state::header_transfering && offset_ >= sizeof(header)) {
            state_ = package_state::body_transfering;
        }
        if (state_ == package_state::body_transfering && offset_ >= sizeof(header) + header_.size) {
            state_ = package_state::ready;
            success_();
        }
    }

private:
    [[nodiscard]] constexpr size_t left_to_write() const noexcept {
        switch (state_) {
        case package_state::header_transfering:
        case package_state::body_transfering:
            return sizeof(header) + header_.size - offset_;
        case package_state::ready:
            return 0;
        }
        return 0;
    }

    std::function<void()> success_;
    std::function<void(std::string_view)> error_;

    std::array<std::byte, BUFFER_SIZE> buffer_;
    size_t offset_;
    header header_;

    package_state state_;
};

class package_reciever {
public:
    package_reciever() : success_{}, error_{}, buffer_{}, offset_{0}, header_{}, state_{package_state::header_transfering} {}

    void on_success(std::function<void(std::variant<message, sign_in, sign_up> &&)> success) { success_ = std::move(success); }

    void on_error(std::function<void(std::string_view)> error) { error_ = std::move(error); }

    void send_error(std::string_view error) { error_(error); }

    [[nodiscard]] auto buffer() {
        if (left_to_read() != 0)
            std::cout << "read " << left_to_read() << std::endl;
        return asio::buffer(buffer_.data() + offset_, std::min(left_to_read(), BUFFER_SIZE - offset_));
    }

    void data_transferred(size_t bytes_transferred) {
        if (bytes_transferred != 0)
            std::cout << "read-transfered " << bytes_transferred << std::endl;
        offset_ += bytes_transferred;
        size_t offset_new = 0;
        while (true) {
            if (state_ == package_state::header_transfering) {
                if (bytes_transferred != 0)
                    std::cout << "header_transferring" << std::endl;
                if (offset_ - offset_new >= sizeof(header)) {
                    if (bytes_transferred != 0)
                        std::cout << "header_read" << std::endl;
                    if (size_t add = header_deserializer(buffer_.data() + offset_new, header_); add == 0) {
                        error_("Unknown type");
                        return;
                    } else {
                        offset_new += add;
                    }
                    if (header_.size > BUFFER_SIZE) {
                        error_("Package is too big");
                        return;
                    }
                    if (bytes_transferred != 0)
                        std::cout << "header_info " << header_.size << std::endl;
                    state_ = package_state::body_transfering;
                } else {
                    break;
                }
            }
            if (state_ == package_state::body_transfering && offset_ - offset_new >= header_.size) {
                std::variant<message, sign_in, sign_up> tmp;
                switch (header_.type) {
                case MESSAGE: {
                    if (size_t add = message_deserializer(buffer_.data() + offset_new, std::get<message>(tmp)); add == 0) {
                        error_("Unknown type");
                        return;
                    } else {
                        offset_new += add;
                    }
                    break;
                }
                case SIGN_IN:
                    offset_new += sign_in_deserializer(buffer_.data() + offset_new, std::get<sign_in>(tmp));
                    break;

                case SIGN_UP:
                    offset_new += sign_up_deserializer(buffer_.data() + offset_new, std::get<sign_up>(tmp));
                    break;
                }
                success_(std::move(tmp));
                state_ = package_state::header_transfering;
            } else {
                break;
            }
        }
        if (offset_new != 0) {
            for (size_t i = offset_new, j = 0; i < offset_; ++i, ++j) {
                buffer_[j] = buffer_[i];
            }
            offset_ -= offset_new;
        }
    }

private:
    [[nodiscard]] constexpr size_t left_to_read() const noexcept {
        switch (state_) {
        case package_state::header_transfering:
            return sizeof(header) - offset_;
        case package_state::body_transfering:
            return header_.size - offset_;
        case package_state::ready:
            return 0;
        }
        return 0;
    }

    std::function<void(std::variant<message, sign_in, sign_up> &&)> success_;
    std::function<void(std::string_view)> error_;

    std::array<std::byte, BUFFER_SIZE> buffer_;
    size_t offset_;
    header header_;

    package_state state_;
};

#endif  // ASYNCSC_PACKAGE_H
