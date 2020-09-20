#ifndef TINYHTTP_TINYHTTP_EPOLL_WORKER_H
#define TINYHTTP_TINYHTTP_EPOLL_WORKER_H

#include "tinyhttp/connection.h"
#include "tinyhttp/server.h"
#include "unixprimwrap/descriptor.h"
#include "trivilog/base_logger.h"

#include <cinttypes>
#include <chrono>
#include <map>

extern "C" {
#include <sys/epoll.h>
}

namespace tinyhttp {

class EpollWorker {
  public:

    // This is event loop configuration structure, for epoll and other stuff
    struct EventLoopConfig {
        // cppcheck-suppress unusedStructMember
        int epoll_max_events = 4096;
        // cppcheck-suppress unusedStructMember
        uint32_t epoll_accept_flags = EPOLLIN;
        // cppcheck-suppress unusedStructMember
        sigset_t *epoll_sigmask = nullptr;
        // cppcheck-suppress unusedStructMember
        int epoll_timeout = -1;
    };

    explicit EpollWorker(int worker_id, const Server &server);

    EpollWorker(const EpollWorker &) = delete;

    EpollWorker &operator=(const EpollWorker &) = delete;

    // TODO(nickeskov): create move constructor and operator
    //EpollWorker(EpollWorker &&other) noexcept = default;
    //EpollWorker &operator=(EpollWorker &&) noexcept = default;


    void event_loop(const EventLoopConfig &cfg);

    void operator()(const EventLoopConfig &cfg);

    ~EpollWorker() = default;

  private:

    using basic_io_service_t = int;

    struct Client {
        Connection connection;
        // cppcheck-suppress unusedStructMember
        uint32_t current_events;
        //std::chrono::high_resolution_clock::time_point timeout;
    };

    const int worker_id_;
    const Server &server_;
    trivilog::BaseLogger &logger_;
    unixprimwrap::Descriptor epoll_fd_;
    std::map<basic_io_service_t, Client> clients_;

    bool add_to_event_loop(Connection &&connection, uint32_t events);

//    bool remove_from_event_loop(Connection &connection);

    bool change_event(Client& client, uint32_t epoll_events);

    void close_connection(basic_io_service_t basic_io_service);

    void accept_connections(size_t max_count);

    void handle_client(epoll_event fd_event);

    void client_routine(); // coroutine function

};

}

#endif //TINYHTTP_TINYHTTP_EPOLL_WORKER_H
