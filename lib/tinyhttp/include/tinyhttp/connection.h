#ifndef TINYHTTP_TINYHTTP_CONNECTION_H
#define TINYHTTP_TINYHTTP_CONNECTION_H

#include <string_view>
#include <string>
#include <cinttypes>

#include "unixprimwrap/descriptor.h"

namespace tinyhttp {

class Connection {
  public:

    Connection(std::string_view ip, uint16_t port, bool set_nonblock = true);

    Connection(const Connection &) = delete;

    Connection &operator=(const Connection &) = delete;

    Connection(Connection &&) = default;

    Connection &operator=(Connection &&) = default;

    [[nodiscard]] const std::string &get_dst_addr() const noexcept;

    [[nodiscard]] const std::string &get_src_addr() const noexcept;

    [[nodiscard]] uint16_t get_dst_port() const noexcept;

    [[nodiscard]] uint16_t get_src_port() const noexcept;

    [[nodiscard]] const unixprimwrap::Descriptor &get_io_service() const noexcept;

    [[nodiscard]] std::string &get_io_buffer() noexcept;

    [[nodiscard]] const std::string &get_io_buffer() const noexcept;

    void set_nonblock(bool opt);

    [[nodiscard]] std::string to_string() const;

    ssize_t read(void *buf, size_t len);

    ssize_t write(const void *buf, size_t len);

    ssize_t read_in_io_buff(size_t len);

    ssize_t write_from_io_buff(size_t len);

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

    std::string io_buffer_;

    std::string tmp_read_buffer_;

    friend class Server;
    friend class EpollWorker;

    Connection(unixprimwrap::Descriptor &&endpoint, std::string &&dst_addr, uint16_t dst_port);

    void set_src_endpoint();
};

}

#endif //TINYHTTP_TINYHTTP_CONNECTION_H
