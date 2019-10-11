#include <memory>
#include "asio.hpp"

class tcp_client: public std::enable_shared_from_this<tcp_client> {
    asio::ip::tcp::socket socket_;

    explicit tcp_client(asio::io_context &io_context) : socket_{io_context} {}

    static void handle_send(const asio::error_code &, size_t) {}

public:
    typedef std::shared_ptr<tcp_client> shared_pointer;

    static shared_pointer create(asio::io_context &io_context) { return shared_pointer{new tcp_client{io_context}}; }

    void send(std::string const &message) {
        asio::async_write(socket_, asio::buffer(message), &tcp_client::handle_send);
    }

    asio::ip::tcp::socket &socket() { return socket_; }
};

class tcp_server {
    asio::ip::tcp::acceptor acceptor_;

    void start_accept() {
        tcp_client::shared_pointer new_connection =
            tcp_client::create(static_cast<asio::io_context &>(acceptor_.get_executor().context()));
        acceptor_.async_accept(new_connection->socket(),
                               std::bind(&tcp_server::handle_accept, this, new_connection, std::placeholders::_1));
    }

    void handle_accept(tcp_client::shared_pointer new_connection, const asio::error_code &error) {
        if (!error) {
            new_connection->send("Lol");
        }
        start_accept();
    }

public:
    tcp_server(asio::io_context &io_context, unsigned short port)
        : acceptor_{io_context, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), port}} {
        start_accept();
    }
};

int main() {
    asio::io_context io_context;
    tcp_server server{io_context, 56789};
    io_context.run();
}