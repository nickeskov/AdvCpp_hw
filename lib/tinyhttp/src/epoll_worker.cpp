#include "tinyhttp/epoll_worker.h"
#include "tinyhttp/errors.h"
#include "tinyhttp/utils.h"
#include "tinyhttp/constants.h"
#include "tinyhttp/http_request_line.h"
#include "tinyhttp/http_headers.h"
#include "tinyhttp/http_request.h"
#include "coroutine/coroutine.h"

#include <string>
#include <vector>
#include <cerrno>
#include <cstring>

extern "C" {
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}

namespace tinyhttp {

using namespace std::literals::string_literals;

namespace {

int epoll_act(int epoll_fd, int act, int fd, uint32_t events);

int epoll_add(int epoll_fd, int fd, uint32_t events);

int epoll_mod(int epoll_fd, int fd, uint32_t new_events);

//int epoll_del(int epoll_fd, int fd) {
//    return epoll_act(epoll_fd, EPOLL_CTL_DEL, fd, 0);
//}

constexpr uint32_t ACCEPTOR_EVENTS = EPOLLIN | EPOLLEXCLUSIVE;
constexpr size_t MAX_ACCEPTIONS_PER_LOOP = 1;
constexpr size_t MAX_READ_BYTES_PER_CALL = 512;

std::string_view read_body(Connection &connection, size_t start_pos, ssize_t body_len);

size_t read_until_headers_end(Connection &connection);

HttpRequest read_http_request(Connection &connection);

}


EpollWorker::EpollWorker(int worker_id, const Server &server)
        : worker_id_(worker_id), server_(server), logger_(server.get_logger()), epoll_fd_(epoll_create(1)) {

    if (!epoll_fd_.is_valid()) {
        throw errors::EpollCreateError("cannot create epoll entity: "s + strerror(errno));
    }

    // add server socket to epoll
    const int srv_status = epoll_add(epoll_fd_.data(),
                                     server_.get_acceptor_service().data(),
                                     ACCEPTOR_EVENTS);

    if (srv_status < 0) {
        throw errors::EpollAddError(
                "cannot add to epoll server socket: "s + std::strerror(errno));
    }
}

bool EpollWorker::add_to_event_loop(Connection &&connection, uint32_t events) {
    auto conn_io_service = connection.get_io_service().data();

    auto client = Client{std::move(connection), events};

    try {
        clients_.emplace(conn_io_service, std::move(client));
    } catch (std::exception &) {
        // Fallback if emplace fails
        connection = std::move(client.connection);
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

bool EpollWorker::change_event(EpollWorker::Client &client, uint32_t epoll_events) {
    int conn_io_service = client.connection.get_io_service().data();

    if (epoll_fd_.is_valid()
        && epoll_mod(epoll_fd_.data(), conn_io_service, epoll_events) < 0) {
        return false;
    }

    client.current_events = epoll_events;
    return true;
}

void EpollWorker::close_connection(EpollWorker::basic_io_service_t basic_io_service) {
    const auto &client = clients_.at(basic_io_service);

    const auto dst_addr = client.connection.dst_addr_;
    const auto dst_port = client.connection.dst_port_;

    clients_.erase(basic_io_service);
    logger_.info("Disconnect with "s + dst_addr + ":" + std::to_string(dst_port)
                 + " [io_service=" + std::to_string(basic_io_service) + "]");
}

void EpollWorker::accept_connections(size_t max_count) {
    sockaddr_in client_addr{};
    socklen_t addr_size = sizeof(client_addr);

    const int basic_acceptor_service = server_.get_acceptor_service().data();

    for (size_t i = 0; i < max_count && !server_.is_stopped(); ++i) {
        unixprimwrap::Descriptor client_fd{
                ::accept(
                        basic_acceptor_service,
                        reinterpret_cast<sockaddr *>(&client_addr),
                        &addr_size
                )
        };

        if (!client_fd.is_valid()) {
            if (errno == EINTR) {
                continue;
            } else {
                bool would_block = errno == EAGAIN || errno == EWOULDBLOCK;
                if (would_block) {
                    return;
                } else {
                    throw errors::AcceptError(
                            "cannot accept new connection, server_sock_fd="
                            + std::to_string(basic_acceptor_service));
                }
            }
        }
        if (utils::set_nonblock(client_fd.data(), true) < 0) {
            throw errors::AcceptError(
                    "cannot cannot set nonblock for new connection, server_sock_fd="
                    + std::to_string(basic_acceptor_service));
        }

        char buff[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &client_addr.sin_addr, buff, INET_ADDRSTRLEN);

        std::string dst_addr = buff;
        uint32_t dst_port = ntohs(client_addr.sin_port);

        const basic_io_service_t client_conn_io_service = client_fd.data();

        auto connection = Connection(std::move(client_fd), std::move(dst_addr), dst_port);

        const std::string client_dst_addr = connection.get_dst_addr();
        const uint32_t client_dst_port = connection.get_dst_port();

        add_to_event_loop(std::move(connection), EPOLLIN);

        coroutine::create(client_conn_io_service, &EpollWorker::client_routine, this);

        logger_.info("Accepted new connection from "s + client_dst_addr + ":" +
                     std::to_string(client_dst_port) +
                     +" [io_service=" + std::to_string(client_conn_io_service) + "]");
    }
}

void EpollWorker::event_loop(const EventLoopConfig &cfg) {
    std::vector<struct epoll_event> fd_events(cfg.epoll_max_events);

    const int basic_acceptor_service = server_.get_acceptor_service().data();

    while (!server_.is_stopped()) {
        const int loop_events_count = epoll_pwait(epoll_fd_.data(),
                                                  fd_events.data(),
                                                  cfg.epoll_max_events,
                                                  cfg.epoll_timeout,
                                                  cfg.epoll_sigmask);

        if (loop_events_count < 0) {
            if (errno != EINTR) {
                throw errors::EpollWaitError(
                        "error epoll_wait_error: "s + std::strerror(errno));
            }
            continue;
        }

        for (int i = 0; i < loop_events_count && !server_.is_stopped(); ++i) {
            const struct epoll_event fd_event = fd_events[i];

            if (fd_event.data.fd == basic_acceptor_service) {
                accept_connections(MAX_ACCEPTIONS_PER_LOOP);
            } else if (fd_event.events & EPOLLHUP || fd_event.events & EPOLLERR) {
                close_connection(fd_event.data.fd);
            } else {
                handle_client(fd_event);
            }
        }
    }
}

void EpollWorker::operator()(const EpollWorker::EventLoopConfig &cfg) {
    event_loop(cfg);
}

void EpollWorker::handle_client(epoll_event fd_event) {
    const auto client_conn_io_service = fd_event.data.fd;

    if (fd_event.events & EPOLLHUP || fd_event.events & EPOLLERR) {
        close_connection(client_conn_io_service);
    }

//    Client &client = clients_.at(client_conn_io_service);

    coroutine::resume_statuses status;
    try {
        status = coroutine::resume(client_conn_io_service);
    } catch (...) {
        // TODO(nickeckov): ignored
    }

    if (status == coroutine::resume_statuses::FINISHED) {
        close_connection(client_conn_io_service);
    }
}

void EpollWorker::client_routine() {
    auto client_conn_io_service = coroutine::current();

    Client &client = clients_.at(client_conn_io_service);

    Connection &connection = client.connection;

    while (true) {
        HttpRequest request = read_http_request(connection);

        connection.get_io_buffer().clear();

        connection.get_io_buffer() += request.get_body();

        connection.write_from_io_buff(request.get_body().size());

        // TODO(nickeskov): create http response

        // TODO(nickeskov): process client request and send response cycle
        break;
    }
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

std::string_view read_body(Connection &connection, size_t start_pos, ssize_t body_len) {
    if (body_len > 0) {
        auto &buffer = connection.get_io_buffer();

        auto buff_size = buffer.size();

        ssize_t was_read = buff_size - start_pos;
        while (was_read < body_len) {
            ssize_t bytes = connection.read_in_io_buff(MAX_READ_BYTES_PER_CALL);

            if (bytes < 0) {
                coroutine::yield();
                continue;
            }

            was_read += bytes;

            coroutine::yield();
        }

        return std::string_view(buffer).substr(start_pos, body_len);
    }

    return {};
}

size_t read_until_headers_end(Connection &connection) {
    auto &buffer = connection.get_io_buffer();

    size_t headers_end_pos = std::string::npos;

    while (true) {
        size_t pos = 0;
        if (buffer.size() > constants::strings::headers_end.size()) {
            pos = buffer.size() - constants::strings::headers_end.size();
        }

        ssize_t bytes = connection.read_in_io_buff(MAX_READ_BYTES_PER_CALL);
        if (bytes < 0) {
            coroutine::yield();
            continue;
        }

        headers_end_pos = buffer.find(constants::strings::headers_end, pos);
        if (headers_end_pos != std::string::npos) {
            break;
        }

        coroutine::yield();
    }

    return headers_end_pos;
}

HttpRequest read_http_request(Connection &connection) {
    size_t headers_end_pos = read_until_headers_end(connection);

    std::string_view buff = connection.get_io_buffer();
    auto newline_pos = buff.find(constants::strings::newline);

    auto request_line_str = buff.substr(0, newline_pos);
    auto headers_str = buff.substr(newline_pos + constants::strings::newline.size(),
                                   headers_end_pos - newline_pos);

    HttpRequestLine request_line(request_line_str);
    HttpHeaders headers(headers_str);

    std::string_view body;

    if (headers.contains(constants::headers::content_length)) {
        size_t start_pos = headers_end_pos + constants::strings::headers_end.size();
        ssize_t body_len = std::stoll(headers.at(constants::headers::content_length));

        // TODO(nickeskov): check if body size lower than zero (<0)

        body = read_body(connection, start_pos, body_len);
    }

    return HttpRequest(std::move(request_line), std::move(headers), body);
}

}

}
