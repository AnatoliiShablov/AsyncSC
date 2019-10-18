#ifndef ASYNCSC_PACKAGE_H
#define ASYNCSC_PACKAGE_H

#include <deque>
#include <stdexcept>
#include "asio.hpp"
#include "asio/write.hpp"
#include "asio/read.hpp"

class package {
    std::deque<std::byte> data_;

    template<typename ReadHandler>
    static void async_save_package(package &pack, std::vector<std::byte> const &buffer,
                                   ReadHandler &&handler, const asio::error_code &error) {

        if (!error) {
            pack.data_.assign(buffer.begin(), buffer.end());
        }
        handler(error);
    }


    template<typename AsyncReadStream, typename ReadHandler>
    static void async_read_package(AsyncReadStream &s, package &pack, std::array<std::byte, 4> len_buf,
                                   ReadHandler &&handler, const asio::error_code &error) {
        if (!error) {
            uint32_t pack_length = static_cast<uint32_t>(len_buf[0]) << UINT32_C(24) |
                                   static_cast<uint32_t>(len_buf[1]) << UINT32_C(16) |
                                   static_cast<uint32_t>(len_buf[2]) << UINT32_C(8) |
                                   static_cast<uint32_t>(len_buf[3]) << UINT32_C(0);
            std::vector<std::byte> buffer(pack_length);
            asio::async_read(s, asio::buffer(buffer),
                             std::bind(&package::async_save_package, pack, buffer, handler, std::placeholders::_1));
        } else {
            handler(error);
        }
    }

public:
    package() : data_{4, std::byte{0}} {}

    friend package &operator<<(package &pack, char object) {
        pack.data_.push_back(static_cast<std::byte>(object));
        return pack;
    }

    friend package &operator>>(package &pack, char &object) {
        if (pack.data_.empty()) {
            throw std::runtime_error("Not enough bytes");
        }
        object = static_cast<char>(pack.data_.front());
        pack.data_.pop_front();
        return pack;
    }

    friend package &operator<<(package &pack, uint32_t object) {
        pack.data_.push_back(static_cast<std::byte>((object & UINT32_C(0xff000000)) >> UINT32_C(24)));
        pack.data_.push_back(static_cast<std::byte>((object & UINT32_C(0x00ff0000)) >> UINT32_C(16)));
        pack.data_.push_back(static_cast<std::byte>((object & UINT32_C(0x0000ff00)) >> UINT32_C(8)));
        pack.data_.push_back(static_cast<std::byte>((object & UINT32_C(0x000000ff)) >> UINT32_C(0)));
        return pack;
    }


    friend package &operator<<(package &pack, std::string const &object) {
        pack << static_cast<uint32_t>(object.length());
        for (auto ch : object) {
            pack << ch;
        }
        return pack;
    }

    friend package &operator>>(package &pack, std::string &object) {
        if (pack.data_.size() < 4) {
            throw std::runtime_error("Not enough bytes");
        }
        uint32_t length;
        pack >> length;
        if (pack.data_.size() < length) {
            throw std::runtime_error("Not enough bytes");
        }
        for (size_t i = 0; i < length; ++i) {
            char ch;
            pack >> ch;
            object.push_back(ch);
        }
        return pack;
    }

    friend package &operator>>(package &pack, uint32_t &object) {
        if (pack.data_.size() < 4) {
            throw std::runtime_error("Not enough bytes");
        }
        object = static_cast<uint32_t>(pack.data_[0]) << UINT32_C(24) |
                 static_cast<uint32_t>(pack.data_[1]) << UINT32_C(16) |
                 static_cast<uint32_t>(pack.data_[2]) << UINT32_C(8) |
                 static_cast<uint32_t>(pack.data_[3]) << UINT32_C(0);
        pack.data_.erase(pack.data_.begin(), pack.data_.begin() + 4);
        return pack;
    }

//    template<typename T>
//    friend package &operator<<(package &pack, T data) = delete;
//
//    template<typename T>
//    friend package &operator>>(package &pack, T &data) = delete;

    void clear() noexcept {
        data_.clear();
    }

    [[nodiscard]] bool empty() const noexcept {
        return data_.empty();
    }

    [[nodiscard]] uint32_t size() const noexcept {
        return data_.size();
    }

    void encrypt() {
        // Do nothing
    }

    void decrypt() {
        // Do nothing
    }

    template<typename SyncReadStream>
    static void read(SyncReadStream &s, package &pack) {
        uint32_t pack_length{0};
        std::vector<std::byte> buffer{4};
        asio::read(s, asio::buffer(buffer));
        pack_length = static_cast<uint32_t>(buffer[0]) << UINT32_C(24) |
                      static_cast<uint32_t>(buffer[1]) << UINT32_C(16) |
                      static_cast<uint32_t>(buffer[2]) << UINT32_C(8) |
                      static_cast<uint32_t>(buffer[3]) << UINT32_C(0);
        buffer.resize(pack_length);
        asio::read(s, asio::buffer(buffer));
        pack.data_.assign(buffer.begin(), buffer.end());
    }

    template<typename SyncWriteStream>
    static void write(SyncWriteStream &s, package &pack) {
        std::vector<std::byte> buffer{4};
        uint32_t pack_length{static_cast<uint32_t>(pack.data_.size())};
        buffer[0] = static_cast<std::byte>((pack_length & UINT32_C(0xff000000)) >> UINT32_C(24));
        buffer[1] = static_cast<std::byte>((pack_length & UINT32_C(0x00ff0000)) >> UINT32_C(16));
        buffer[2] = static_cast<std::byte>((pack_length & UINT32_C(0x0000ff00)) >> UINT32_C(8));
        buffer[3] = static_cast<std::byte>((pack_length & UINT32_C(0x000000ff)) >> UINT32_C(0));
        asio::write(s, asio::buffer(buffer));
        buffer.assign(pack.data_.begin(), pack.data_.end());
        asio::write(s, asio::buffer(buffer));
    }

    template<typename AsyncReadStream, typename ReadHandler>
    static void async_read(AsyncReadStream &s, package &pack, ReadHandler &&handler) {
        std::array<std::byte, 4> len_buf{};
        asio::async_read(s, asio::buffer(len_buf),
                         std::bind(&package::async_read_package, s, pack, len_buf, handler, std::placeholders::_1));
    }

    template<typename AsyncWriteStream, typename WriteHandler>
    static void async_write(AsyncWriteStream &s, package &pack, WriteHandler &&handler) {
        std::vector<std::byte> buffer{pack.data_.size() + 4};
        uint32_t pack_length{static_cast<uint32_t>(pack.data_.size())};
        buffer[0] = static_cast<std::byte>((pack_length & UINT32_C(0xff000000)) >> UINT32_C(24));
        buffer[1] = static_cast<std::byte>((pack_length & UINT32_C(0x00ff0000)) >> UINT32_C(16));
        buffer[2] = static_cast<std::byte>((pack_length & UINT32_C(0x0000ff00)) >> UINT32_C(8));
        buffer[3] = static_cast<std::byte>((pack_length & UINT32_C(0x000000ff)) >> UINT32_C(0));
        std::copy(pack.data_.begin(), pack.data_.end(), buffer.begin() + 4);
        asio::async_write(s, asio::buffer(buffer), std::bind(handler, std::placeholders::_1));
    }
};


#endif //ASYNCSC_PACKAGE_H
