#ifndef TINYHTTP_TINYHTTP_SERVER_H
#define TINYHTTP_TINYHTTP_SERVER_H

#include <string_view>
#include <thread>
#include <atomic>
#include <chrono>

#include "unixprimwrap/descriptor.h"
#include "trivilog/base_logger.h"

namespace tinyhttp {

class Server {
  public:

    Server(std::string_view ip, uint16_t port, trivilog::BaseLogger &logger);

    Server(const Server &) = delete;

    Server &operator=(const Server &) = delete;

    // TODO(nickeskov): create move constructor and operator
    //Server(Server &&) noexcept;
    //Server &operator=(Server &&) noexcept;

    void run(size_t thread_counts = std::thread::hardware_concurrency() - 1);

    // TODO(nickeskov): create on_request method for Server

    [[nodiscard]] const std::string &get_src_addr() const noexcept;

    [[nodiscard]] uint16_t get_src_port() const noexcept;

    [[nodiscard]] trivilog::BaseLogger &get_logger() const noexcept;

    [[nodiscard]] const unixprimwrap::Descriptor &get_acceptor_service() const noexcept;

    [[nodiscard]] bool is_opened() const noexcept;

    [[nodiscard]] bool is_stopped() const noexcept;

    void stop() noexcept;

    void close();

    virtual ~Server() noexcept = default;

  protected:
    unixprimwrap::Descriptor server_sock_fd_;

    trivilog::BaseLogger &logger_;

    std::atomic<bool> is_stopped_ = false;

  private:
    std::string src_addr_;
    uint16_t src_port_{};
};

}

#endif //TINYHTTP_TINYHTTP_SERVER_H
