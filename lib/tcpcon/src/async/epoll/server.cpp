#include "tcpcon/async/epoll/server.h"
#include "tcpcon/async/utils.h"
#include "tcpcon/errors.h"

#include <string>
#include <vector>
#include <utility>

extern "C" {
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}

namespace {

int epoll_act(int epoll_fd, int act, int fd, uint32_t events) {
    struct epoll_event epoll_event{};
    epoll_event.events = events;
    epoll_event.data.fd = fd;
    return epoll_ctl(epoll_fd, act, fd, &epoll_event);
}

int epoll_add(int epoll_fd, int fd, uint32_t events) {
    return epoll_act(epoll_fd, EPOLL_CTL_ADD, fd, events);
}

int epoll_mod(int epoll_fd, int fd, uint32_t new_events) {
    return epoll_act(epoll_fd, EPOLL_CTL_MOD, fd, new_events);
}

int epoll_del(int epoll_fd, int fd) {
    return epoll_act(epoll_fd, EPOLL_CTL_DEL, fd, 0);
}

}

namespace tcpcon::async::ipv4 {

Server::Server(std::string_view ip, uint16_t port)
        : server_sock_fd_(socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)) {

    if (!server_sock_fd_.is_valid()) {
        throw errors::IoServiceError("cannot create IPV4 socket");
    }

    int yes = 1;
    int set_reuseaddr_opt_status = setsockopt(server_sock_fd_.data(),
                                              SOL_SOCKET,
                                              SO_REUSEADDR,
                                              &yes, sizeof(yes));
    if (set_reuseaddr_opt_status < 0) {
        throw errors::IoServiceError("cannot set SO_REUSEADDR to IPV4 socket");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(addr.sin_family, ip.data(), &addr.sin_addr) == 0) {
        std::string msg = "invalid ip address, ip=";
        msg.append(ip);
        throw errors::InvalidAddressError(msg);
    }

    int bind_status = ::bind(server_sock_fd_.data(),
                             reinterpret_cast<sockaddr *>(&addr),
                             sizeof(addr));
    if (bind_status < 0) {
        std::string msg = "cannot bind to addr=";
        msg += ip;
        msg += ", port=";
        msg += std::to_string(port);
        throw errors::BindError(msg);
    }

    if (::listen(server_sock_fd_.data(), SOMAXCONN) < 0) {
        std::string msg = "cannot start listen on addr=";
        msg.append(ip) += ", port=" + std::to_string(port);
        throw errors::ListenError(msg);
    }

    if (port == 0) {
        socklen_t addr_size = sizeof(addr);
        int getsockname_status = getsockname(server_sock_fd_.data(),
                                             reinterpret_cast<sockaddr *>(&addr),
                                             &addr_size);
        if (getsockname_status < 0) {
            throw errors::IoServiceError(
                    "cannot get info about self, server_sock_fd="
                    + std::to_string(server_sock_fd_.data()));
        }
        port = ntohs(addr.sin_port);
    }

    src_addr_ = ip;
    src_port_ = port;
}

Server::Server(Server &&other) noexcept {
    server_sock_fd_ = std::move(other.server_sock_fd_);
    src_addr_ = std::move(other.src_addr_);
    src_port_ = other.src_port_;
    clients_ = std::move(other.clients_);
    is_stoped_.exchange(other.is_stoped_);
}

Server &Server::operator=(Server &&other) noexcept {
    if (this == &other) {
        return *this;
    }

    server_sock_fd_ = std::move(other.server_sock_fd_);
    src_addr_ = std::move(other.src_addr_);
    src_port_ = other.src_port_;
    clients_ = std::move(other.clients_);
    is_stoped_.exchange(other.is_stoped_);

    other.src_port_ = 0;
    return *this;
}

const std::string &Server::get_src_addr() const noexcept {
    return src_addr_;
}

uint16_t Server::get_src_port() const noexcept {
    return src_port_;
}

bool Server::is_opened() const noexcept {
    return server_sock_fd_.is_valid();
}

void Server::close_connections(uint32_t close_type) {
    for (auto &fd_conn_pair : clients_) {
        close_connection(fd_conn_pair.second.connection, close_type);
    }
}

void Server::stop() noexcept {
    is_stoped_ = true;
}

void Server::close(uint32_t close_type) {
    if (is_opened()) {
        close_connections(close_type);
        if (server_sock_fd_.close() < 0) {
            throw errors::ServerCloseError(
                    "error while closing server socket, server_sock_fd="
                    + std::to_string(server_sock_fd_.data()));
        }
    }
}

bool Server::add_to_event_loop(Connection &&connection, uint32_t events) {
    int conn_io_service = connection.get_io_service().data();

    auto io_service_conn_pair = ConnectionWithEvent{std::move(connection), events};

    try {
        clients_.emplace(conn_io_service, std::move(io_service_conn_pair));
    } catch (std::exception &) {
        // Fallback if emplace fails
        connection = std::move(io_service_conn_pair.connection);
        throw;
    }

    if (epoll_fd_.is_valid()
        && epoll_add(epoll_fd_.data(), conn_io_service, events) < 0) {
        // Fallback if epoll_ctl fails
        connection = std::move(clients_.at(conn_io_service).connection);
        clients_.erase(conn_io_service);
        return false;
    }
    return true;
}

bool Server::remove_from_event_loop(Connection &connection) {
    int conn_io_service = connection.get_io_service().data();

    if (epoll_fd_.is_valid()
        && epoll_del(epoll_fd_.data(), conn_io_service) < 0) {
        return false;
    }
    // Not throws any exceptions, because key type == int
    clients_.erase(conn_io_service);
    return false;
}

bool Server::change_event(const Connection &connection, uint32_t epoll_events) {
    int conn_io_service = connection.get_io_service().data();

    if (epoll_fd_.is_valid()
        && epoll_mod(epoll_fd_.data(), conn_io_service, epoll_events) < 0) {
        return false;
    }

    clients_.at(conn_io_service).events = epoll_events;
    return true;
}

void Server::event_loop(const connection_handler_t &handler, const EventLoopConfig &cfg) {
    if (!handler) {
        throw errors::BadHandlerError("event_loop bad handler, server_sock_fd="
                                      + std::to_string(server_sock_fd_.data()));
    }

    is_stoped_ = false;

    epoll_fd_ = unixprimwrap::Descriptor{epoll_create(1)};
    if (!epoll_fd_.is_valid()) {
        throw errors::EpollCreateError("cannot create epoll entity");
    }

    // add server socket to epoll
    int server_sock_add_status = ::epoll_add(epoll_fd_.data(),
                                             server_sock_fd_.data(),
                                             cfg.epoll_server_flags | EPOLLIN);
    if (server_sock_add_status < 0) {
        std::string msg = "cannot add to epoll server socket, server_sock_fd=";
        msg += std::to_string(server_sock_fd_.data());
        msg += ", event=";
        msg += std::to_string(cfg.epoll_server_flags | EPOLLIN);

        throw errors::EpollAddError(msg);
    }

    // add prepared connections to epoll
    for (const auto &[conn_io_service, conn_with_event] : clients_) {
        if (epoll_add(epoll_fd_.data(), conn_io_service, conn_with_event.events) < 0) {
            std::string msg = "cannot add to epoll connection with sock_fd=";
            msg += std::to_string(conn_with_event.connection.get_io_service().data());
            msg += ", event=";
            msg += std::to_string(conn_with_event.events);

            throw errors::EpollAddError(msg);
        }
    }

    // start event loop
    std::vector<struct epoll_event> fd_events(cfg.epoll_max_events);
    while (!is_stoped_) {
        int loop_events_count = epoll_pwait(epoll_fd_.data(),
                                            fd_events.data(),
                                            cfg.epoll_max_events,
                                            cfg.epoll_timeout,
                                            cfg.epoll_sigmask);
        if (loop_events_count < 0) {
            if (errno != EINTR) {
                throw errors::EpollWaitError("error epoll_wait_error, server_sock_fd="
                                             + std::to_string(server_sock_fd_.data()));
            }
            continue;
        }

        for (int i = 0; i < loop_events_count && !is_stoped_; ++i) {
            struct epoll_event &received_fd = fd_events[i];

            if (received_fd.data.fd == server_sock_fd_.data()) {
                accept_connections(cfg.epoll_accept_flags,
                                   received_fd.events,
                                   cfg.max_accept_clients_per_loop);
            } else {
                Connection &client = clients_.at(received_fd.data.fd).connection;
                if (received_fd.events & EPOLLHUP
                    || received_fd.events & EPOLLERR) {
                    close_connection(client, received_fd.events);
                } else {
                    try {
                        handler(client, received_fd.events);
                    } catch (...) {
                        close_connection(client, received_fd.events);
                    }
                }
            }
        }
    }

    epoll_fd_.close();
}

void Server::set_after_accept_handler(const connection_handler_t &handler) {
    after_accept_handler_ = handler;
}

void Server::set_before_close_handler(const const_connection_handler_t &handler) {
    before_close_handler_ = handler;
}

void Server::close_connection(Connection &connection, uint32_t events) {
    if (before_close_handler_) {
        try {
            before_close_handler_(connection, events);
        } catch (...) {}
    }
    clients_.erase(connection.get_io_service().data());
}

void Server::accept_connections(uint32_t epoll_accept_flags, uint32_t accept_type, int max_count) {
    sockaddr_in client_addr{};
    socklen_t addr_size = sizeof(client_addr);
    char buff[INET_ADDRSTRLEN];

    while (!is_stoped_ && max_count != 0) {
        unixprimwrap::Descriptor client_fd{
                ::accept(
                        server_sock_fd_.data(),
                        reinterpret_cast<sockaddr *>(&client_addr),
                        &addr_size)
        };

        if (!client_fd.is_valid()) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            } else {
                throw errors::AcceptError("cannot accept new connection, server_sock_fd="
                                          + std::to_string(server_sock_fd_.data()));
            }
        }

        if (utils::set_nonblock(client_fd.data(), true) < 0) {
            throw errors::AcceptError("cannot cannot set nonblock for new connection, server_sock_fd="
                                      + std::to_string(server_sock_fd_.data()));
        }

        inet_ntop(AF_INET, &client_addr.sin_addr, buff, INET_ADDRSTRLEN);

        std::string dst_addr = buff;
        uint32_t dst_port = ntohs(client_addr.sin_port);

        int clients_map_key = client_fd.data();
        add_to_event_loop(Connection(std::move(client_fd), std::move(dst_addr), dst_port),
                          epoll_accept_flags);

        if (after_accept_handler_) {
            // cppcheck-suppress variableScope symbolName=new_connection
            Connection &new_connection = clients_.at(clients_map_key).connection;
            try {
                after_accept_handler_(new_connection, accept_type);
            } catch (...) {}
        }

        if (max_count > 0) {
            --max_count;
        }
    }
}

}
