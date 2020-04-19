#ifndef TCPCON_TCPCON_ASYNC_EPOLL_SERVER_H
#define TCPCON_TCPCON_ASYNC_EPOLL_SERVER_H

#include <map>
#include <atomic>
#include <functional>
#include <csignal>
#include <sys/epoll.h>

#include "tcpcon/async/connection.h"
#include "unixprimwrap/descriptor.h"

namespace tcpcon::async::ipv4 {

using connection_handler_t = std::function<void(Connection & , uint32_t)>;
using const_connection_handler_t = std::function<void(const Connection & , uint32_t)>;

class Server {
  public:
    Server(std::string_view ip, uint16_t port);

    Server(const Server &) = delete;

    Server &operator=(const Server &) = delete;

    Server(Server &&other) noexcept;

    Server &operator=(Server &&other) noexcept;

    [[nodiscard]] const std::string &get_src_addr() const noexcept;

    [[nodiscard]] uint16_t get_src_port() const noexcept;

    void set_after_accept_handler(const connection_handler_t &handler);

    void set_before_close_handler(const const_connection_handler_t &handler);

    bool add_to_event_loop(Connection &&connection, uint32_t events);

    bool remove_from_event_loop(Connection &connection);

    bool change_event(const Connection &connection, uint32_t epoll_events);

    struct EventLoopConfig {
        int epoll_max_events = 4096;
        uint32_t epoll_server_flags = EPOLLIN;
        uint32_t epoll_accept_flags = EPOLLIN;
        int epoll_timeout = -1;
        sigset_t *epoll_sigmask = nullptr;
        int max_accept_clients_per_loop = -1;
    };

    void event_loop(const connection_handler_t &handler, const EventLoopConfig &cfg);

    [[nodiscard]] bool is_opened() const noexcept;

    void close_connection(Connection &connection, uint32_t events);

    void close_connections(uint32_t close_type);

    void stop() noexcept;

    void close(uint32_t close_type);

    ~Server() noexcept = default;

  private:
    unixprimwrap::Descriptor server_sock_fd_;
    unixprimwrap::Descriptor epoll_fd_;

    std::string src_addr;
    uint16_t src_port{};

    struct ConnectionWithEvent {
        Connection connection;
        uint32_t events;
    };
    std::map<int, ConnectionWithEvent> clients_;

    std::atomic<bool> is_stoped_ = false;

    connection_handler_t after_accept_handler_;

    const_connection_handler_t before_close_handler_;

    void accept_connections(uint32_t epoll_accept_flags, uint32_t accept_type, int max_count);
};

}
#endif //TCPCON_TCPCON_ASYNC_EPOLL_SERVER_H
