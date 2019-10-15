#include "lib/tcp_server.h"

int main() {
    tcp_server server(56789);
    server.start_server();
    return 0;
}