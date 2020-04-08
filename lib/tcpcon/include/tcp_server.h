#ifndef HW_TCPCON_TCP_SERVER_H
#define HW_TCPCON_TCP_SERVER_H

#include <string_view>
#include <string>
#include <cinttypes>

#include "descriptor.h"
#include "tcp_connection.h"

namespace tcpcon::ipv4 {

class Server {
  public:

    Server(std::string_view ip, uint16_t port);

    Server(const Server &) = delete;

    Server &operator=(const Server &) = delete;

    Server(Server &&) = default;

    Server &operator=(Server &&) = default;

    [[nodiscard]] const std::string &get_src_addr() const noexcept;

    [[nodiscard]] uint32_t get_src_port() const noexcept;

    Connection accept();

    [[nodiscard]] bool is_opened() const noexcept;

    void close();

    ~Server() noexcept = default;

  private:
    linuxproc::Descriptor server_sock_fd_;

    std::string src_addr;
    uint32_t src_port{};
};

}

#endif //HW_TCPCON_TCP_SERVER_H
