#ifndef HW_TCPCON_TCP_CONNECTION_H
#define HW_TCPCON_TCP_CONNECTION_H

#include <string_view>
#include <string>
#include <cinttypes>

#include "descriptor.h"

namespace tcpcon::ipv4 {

class Connection {
  public:

    Connection(std::string_view ip, uint16_t port);

    Connection(const Connection &) = delete;

    Connection &operator=(const Connection &) = delete;

    Connection(Connection &&) = default;

    Connection &operator=(Connection &&) = default;

    [[nodiscard]] const std::string &get_dst_addr() const noexcept;

    [[nodiscard]] const std::string &get_src_addr() const noexcept;

    [[nodiscard]] uint16_t get_dst_port() const noexcept;

    [[nodiscard]] uint16_t get_src_port() const noexcept;

    [[nodiscard]] std::string to_string() const;

    size_t write(const void *buf, size_t len);

    void write_exact(const void *buf, size_t len);

    size_t read(void *buf, size_t len);

    void read_exact(void *buf, size_t len);

    void connect(std::string_view ip, uint16_t port);

    void set_read_timeout(int seconds);

    void set_write_timeout(int seconds);

    [[nodiscard]] bool is_opened() const noexcept;

    [[nodiscard]] bool is_readable() const noexcept;

    void close();

    ~Connection() noexcept;

  private:
    unixprimwrap::Descriptor sock_fd_;

    std::string dst_addr_;
    std::string src_addr_;

    uint16_t dst_port_{};
    uint16_t src_port_{};

    bool is_readable_ = true;

    friend class Server;

    Connection(unixprimwrap::Descriptor &&endpoint, std::string &&dst_addr, uint16_t dst_port);

    void set_src_endpoint();
};

}

#endif //HW_TCPCON_TCP_CONNECTION_H
