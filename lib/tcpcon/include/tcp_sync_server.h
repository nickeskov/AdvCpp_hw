#ifndef HW_TCPCON_TCP_SERVER_H
#define HW_TCPCON_TCP_SERVER_H

#include <string_view>
#include <string>
#include <cinttypes>

#include "descriptor.h"
#include "tcp_sync_connection.h"

namespace tcpcon::ipv4::sync {

class Server {
  public:

    Server(std::string_view ip, uint16_t port);

    Server(const Server &) = delete;

    Server &operator=(const Server &) = delete;

    Server(Server &&) = default;

    Server &operator=(Server &&) = default;

    [[nodiscard]] const std::string &get_src_addr() const noexcept;

    [[nodiscard]] uint16_t get_src_port() const noexcept;

    Connection accept();

    [[nodiscard]] bool is_opened() const noexcept;

    void close();

    ~Server() noexcept = default;

  private:
    unixprimwrap::Descriptor server_sock_fd_;

    std::string src_addr;
    uint16_t src_port{};
};

}

#endif //HW_TCPCON_TCP_SERVER_H
