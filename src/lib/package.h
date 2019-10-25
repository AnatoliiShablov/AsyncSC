#ifndef ASYNCSC_PACKAGE_H
#define ASYNCSC_PACKAGE_H

#include <deque>
#include <stdexcept>
#include <utility>
#include "asio.hpp"
#include "asio/write.hpp"
#include "asio/read.hpp"

size_t const buffer_size = 64 * 1024 * 1024; // 64 MB

class base_package {
public:
    base_package() = default;

    explicit base_package(std::byte const *buffer, size_t package_size = 0) {
        for (size_t i = 0; i < package_size; ++i) {
            data_.push_back(buffer[i]);
        }
    }

    friend base_package &operator<<(base_package &package, char object) {
        package.data_.push_back(static_cast<std::byte>(object));
        return package;
    }

    friend base_package &operator>>(base_package &package, char &object) {
        if (package.data_.empty()) {
            throw std::runtime_error("Not enough bytes");
        }
        object = static_cast<char>(package.data_.front());
        package.data_.pop_front();
        return package;
    }

    friend base_package &operator<<(base_package &package, uint32_t object) {
        package.data_.push_back(static_cast<std::byte>((object & UINT32_C(0xff000000)) >> UINT32_C(24)));
        package.data_.push_back(static_cast<std::byte>((object & UINT32_C(0x00ff0000)) >> UINT32_C(16)));
        package.data_.push_back(static_cast<std::byte>((object & UINT32_C(0x0000ff00)) >> UINT32_C(8)));
        package.data_.push_back(static_cast<std::byte>((object & UINT32_C(0x000000ff)) >> UINT32_C(0)));
        return package;
    }

    friend base_package &operator>>(base_package &package, uint32_t &object) {
        if (package.data_.size() < 4) {
            throw std::runtime_error("Not enough bytes");
        }
        object = static_cast<uint32_t>(package.data_[0]) << UINT32_C(24) |
                 static_cast<uint32_t>(package.data_[1]) << UINT32_C(16) |
                 static_cast<uint32_t>(package.data_[2]) << UINT32_C(8) |
                 static_cast<uint32_t>(package.data_[3]) << UINT32_C(0);
        package.data_.erase(package.data_.begin(), package.data_.begin() + 4);
        return package;
    }

    friend base_package &operator<<(base_package &package, std::string const &object) {
        package << static_cast<uint32_t>(object.length());
        for (auto ch : object) {
            package << ch;
        }
        return package;
    }

    friend base_package &operator>>(base_package &package, std::string &object) {
        if (package.data_.size() < 4) {
            throw std::runtime_error("Not enough bytes");
        }
        uint32_t length;
        package >> length;
        if (package.data_.size() < length) {
            throw std::runtime_error("Not enough bytes");
        }
        for (size_t i = 0; i < length; ++i) {
            char ch;
            package >> ch;
            object.push_back(ch);
        }
        return package;
    }

    void clear() noexcept {
        data_.clear();
    }

    [[nodiscard]] size_t size() const noexcept {
        return data_.size();
    }

    auto begin() const noexcept {
        return data_.begin();
    }

    auto end() const noexcept {
        return data_.end();
    }

private:
    std::deque<std::byte> data_{};
};

class base_header {
public:
    base_header() : size_{0} {}

    [[nodiscard]] static constexpr size_t header_size() noexcept {
        return sizeof(base_header);
    }

    void write_header(std::byte *buffer) const noexcept {
        buffer[0] = static_cast<std::byte>((size_ & UINT32_C(0xff000000)) >> UINT32_C(24));
        buffer[1] = static_cast<std::byte>((size_ & UINT32_C(0x00ff0000)) >> UINT32_C(16));
        buffer[2] = static_cast<std::byte>((size_ & UINT32_C(0x0000ff00)) >> UINT32_C(8));
        buffer[3] = static_cast<std::byte>((size_ & UINT32_C(0x000000ff)) >> UINT32_C(0));
    }

    void constexpr read_header(std::byte const *buffer) noexcept {
        size_ = static_cast<uint32_t>(buffer[0]) << UINT32_C(24) |
                static_cast<uint32_t>(buffer[1]) << UINT32_C(16) |
                static_cast<uint32_t>(buffer[2]) << UINT32_C(8) |
                static_cast<uint32_t>(buffer[3]) << UINT32_C(0);
    }

    [[nodiscard]] constexpr size_t package_size() const noexcept {
        return size_;
    }

    void set_package_size(size_t new_size) noexcept {
        size_ = static_cast<uint32_t >(new_size);
    }

private:
    uint32_t size_;
};

class package_decoder {
public:
    package_decoder() : success_{}, error_{}, buffer_{}, offset_{0}, state_{state::head_transferring}, tmp_header{} {}

    void on_success(std::function<void(base_package &&)> success) {
        success_ = std::move(success);
    }

    void on_error(std::function<void(std::string_view)> error) {
        error_ = std::move(error);
    }

    void send_error(std::string_view error) {
        error_(error);
    }

    auto buffer() {
        return asio::buffer(buffer_.data() + offset_, std::min(left_to_read(), buffer_size - offset_));
    }

    void data_transferred(size_t bytes_transferred) {
        offset_ += bytes_transferred;
        if (state_ == state::head_transferring && offset_ >= base_header::header_size()) {
            state_ = state::body_transferring;
            tmp_header.read_header(buffer_.data());
            if (tmp_header.package_size() > buffer_size - base_header::header_size()) {
                error_("Message is too big");
            }
        }
        if (left_to_read() == 0) {
            success_(base_package(buffer_.data(), tmp_header.package_size()));
            offset_ = 0;
            state_ = state::head_transferring;
        }
    }

private:
    enum class state {
        head_transferring, body_transferring
    };

    [[nodiscard]] constexpr size_t left_to_read() const noexcept {
        switch (state_) {
            case state::head_transferring :
                return base_header::header_size() - offset_;
            case state::body_transferring :
                return base_header::header_size() + tmp_header.package_size() - offset_;
            default:
                return 0;
        }
    }

    std::function<void(base_package &&)> success_;
    std::function<void(std::string_view)> error_;

    std::array<std::byte, buffer_size> buffer_;
    size_t offset_;
    state state_;

    base_header tmp_header;
};

class package_encoder {
public:
    package_encoder() : success_{}, error_{}, buffer_{}, tmp_header{} {}

    void on_success(std::function<void()> success) {
        success_ = std::move(success);
    }

    void on_error(std::function<void(std::string_view)> error) {
        error_ = std::move(error);
    }

    void send_error(std::string_view error) {
        error_(error);
    }

    void send_success() {
        success_();
    }

    auto buffer() {
        return asio::buffer(buffer_.data(), base_header::header_size() + tmp_header.package_size());
    }

    void encode(base_package const &package) {
        if (package.size() > buffer_size - base_header::header_size()) {
            error_("Message is too big");
        }
        tmp_header.set_package_size(package.size());
        tmp_header.write_header(buffer_.data());
        std::copy(package.begin(), package.end(), buffer_.data() + base_header::header_size());
    }

private:
    std::function<void()> success_;
    std::function<void(std::string_view)> error_;

    std::array<std::byte, buffer_size> buffer_;

    base_header tmp_header;
};

class package_connection {
public :
    package_connection(std::function<void()> success_write_handler,
                       std::function<void(std::string_view)> error_write_handler,
                       std::function<void(base_package &&)> success_read_handler,
                       std::function<void(std::string_view)> error_read_handler,
                       std::string_view host, std::string_view service,
                       asio::io_context &io_context) :
            socket_{io_context}, decoder_{new package_decoder{}}, encoder_{new package_encoder{}} {
        decoder_->on_error(std::move(error_read_handler));
        decoder_->on_success(std::move(success_read_handler));
        encoder_->on_success(std::move(success_write_handler));
        encoder_->on_error(std::move(error_write_handler));

        asio::connect(socket_, asio::ip::tcp::resolver(io_context).resolve(host, service));
        do_read();
    }

    void do_read() {
        socket_.async_read_some(decoder_->buffer(),
                                [this](asio::error_code const &error, size_t bytes_recieved) {
                                    if (error) {
                                        decoder_->send_error(error.message());
                                        return;
                                    }
                                    decoder_->data_transferred(bytes_recieved);
                                    do_read();
                                });
    }

    void do_write() {
        asio::async_write(socket_, encoder_->buffer(),
                          [this](asio::error_code const &error, size_t bytes_sent) {
                              if (error) {
                                  encoder_->send_error(error.message());
                              } else {
                                  encoder_->send_success();
                              }
                          });
    }

private:
    asio::ip::tcp::socket socket_;
    std::unique_ptr<package_decoder> decoder_;
    std::unique_ptr<package_encoder> encoder_;
};

#endif //ASYNCSC_PACKAGE_H
