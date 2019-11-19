#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "lib/connection.h"
#include "lib/package.h"

template <typename T>
struct task_queue {
    std::condition_variable cv;
    std::mutex cv_mutex;

    std::queue<T> tasks;
};

void success_write_handler(std::mutex &err_mutex) {
    static size_t packages_sent = 0;
    std::lock_guard<std::mutex> locker(err_mutex);
    std::fprintf(stderr, "Successfully sent %5zu packages\n", ++packages_sent);
    std::fflush(stderr);
}

void error_write_handler(std::mutex &err_mutex, std::string_view error_message) {
    std::lock_guard<std::mutex> locker(err_mutex);
    std::fprintf(stderr, "Write error: %.*s\n", static_cast<int>(error_message.size()), error_message.data());
    std::fflush(stderr);
}

void success_read_handler(std::mutex &err_mutex, task_queue<std::variant<message, special_signal>> &from_server,
                          std::variant<message, sign_in, sign_up, special_signal> &&package) {
    static size_t packages_recieved = 0;
    {
        std::lock_guard<std::mutex> locker(err_mutex);
        std::fprintf(stderr, "Successfully recieved %5zu packages\n", ++packages_recieved);
        std::fflush(stderr);
    }
    std::lock_guard<std::mutex> q_locker(from_server.cv_mutex);
    from_server.tasks.emplace(std::move(std::get<message>(package)));
    from_server.cv.notify_one();
}

void error_read_handler(std::mutex &err_mutex, std::string_view error_message) {
    std::lock_guard<std::mutex> locker(err_mutex);
    std::fprintf(stderr, "Read error: %.*s\n", static_cast<int>(error_message.size()), error_message.data());
    std::fflush(stderr);
}

void connection_loop(task_queue<std::variant<message, sign_in, sign_up, special_signal>> &to_server,
                     task_queue<std::variant<message, special_signal>> &from_server, asio::io_context &io_context,
                     std::atomic_bool &is_done, std::mutex &err_mutex) {
    client_connection client{
        std::bind(&success_write_handler, std::ref(err_mutex)),
        std::bind(&error_write_handler, std::ref(err_mutex), std::placeholders::_1),
        std::bind(&success_read_handler, std::ref(err_mutex), std::ref(from_server), std::placeholders::_1),
        std::bind(&error_read_handler, std::ref(err_mutex), std::placeholders::_1),
        "localhost",
        "56789",
        io_context};

    while (!is_done.load()) {
        io_context.poll();
        std::unique_lock<std::mutex> locker(to_server.cv_mutex);
        if (to_server.cv.wait_for(locker, std::chrono::milliseconds(1), [&to_server]() { return !to_server.tasks.empty(); })) {
            while (!to_server.tasks.empty()) {
                client.write(to_server.tasks.front());
                to_server.tasks.pop();
            }
        }
    }
}

[[deprecated]] void new_tasks(task_queue<std::variant<message, sign_in, sign_up, special_signal>> &to_server,
                              std::thread *connection, std::atomic_bool &is_done) {
    return;
}

int main() {
    task_queue<std::variant<message, sign_in, sign_up, special_signal>> to_server;
    task_queue<std::variant<message, special_signal>> from_server;
    std::atomic_bool is_done{false};
    std::mutex err_mutex;
    std::mutex out_mutex;

    asio::io_context io_context;
    std::thread connection(&connection_loop, std::ref(to_server), std::ref(from_server), std::ref(io_context),
                           std::ref(is_done), std::ref(err_mutex));

    while (!is_done) {
        new_tasks(to_server, &connection, is_done);
        std::unique_lock<std::mutex> locker(from_server.cv_mutex);
        if (from_server.cv.wait_for(locker, std::chrono::milliseconds(1),
                                    [&from_server]() { return !from_server.tasks.empty(); })) {
            while (!from_server.tasks.empty()) {
                std::lock_guard<std::mutex> locker_out(out_mutex);
                switch (from_server.tasks.front().index()) {
                case 0: {
                    message tmp = std::move(std::get<message>(from_server.tasks.front()));
                    std::cout << tmp.name << " : " << tmp.text << std::endl;
                }
                case 1: {
                    special_signal tmp = std::get<special_signal>(from_server.tasks.front());
                    switch (tmp.type) {
                    case special_signal::NOT_SIGNED:
                        std::cout << "NOT SIGNED" << std::endl;
                        break;
                    case special_signal::ALREADY_SIGNED:
                        std::cout << "ALREADY_SIGNED" << std::endl;
                        break;
                    case special_signal::USER_ALREADY_EXISTS:
                        std::cout << "USER_ALREADY_EXISTS" << std::endl;
                        break;
                    case special_signal::WRONG_PASSWORD:
                        std::cout << "WRONG_PASSWORD" << std::endl;
                        break;
                    case special_signal::WEAK_PASSWORD:
                        std::cout << "WEAK_PASSWORD" << std::endl;
                        break;
                    case special_signal::SIGNED_UP:
                        std::cout << "SIGNED_UP" << std::endl;
                        break;
                    case special_signal::SIGNED_IN:
                        std::cout << "SIGNED_IN" << std::endl;
                        break;
                    case special_signal::SIGN_OUT:
                    case special_signal::QUIT:
                    case special_signal::special_signals_amount:
                        std::cout << "Sorry, but what???" << std::endl;
                        break;
                    }
                }
                }
            }
        }
    }
}
