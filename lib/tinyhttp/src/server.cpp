#include "tinyhttp/server.h"
#include "tinyhttp/errors.h"
#include "tinyhttp/epoll_worker.h"

#include <cerrno>
#include <cstring>
#include <version>
#include <thread>

extern "C" {
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}


namespace tinyhttp {

using namespace std::literals::string_literals;

Server::Server(std::string_view ip, uint16_t port, trivilog::BaseLogger &logger)
        : server_sock_fd_(socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)), logger_(logger) {

    int yes = 1;
    int reuseaddr_status = setsockopt(server_sock_fd_.data(),
                                      SOL_SOCKET,
                                      SO_REUSEADDR,
                                      &yes, sizeof(yes));
    if (reuseaddr_status < 0) {
        throw errors::IoServiceError(
                "cannot set SO_REUSEADDR to IPV4 socket: "s + std::strerror(errno));
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(addr.sin_family, ip.data(), &addr.sin_addr) == 0) {
        std::string msg = "invalid ip address, ip=";
        msg += ip;
        throw errors::InvalidAddressError(msg);
    }

    int bind_status = ::bind(server_sock_fd_.data(),
                             reinterpret_cast<sockaddr *>(&addr),
                             sizeof(addr));
    if (bind_status < 0) {
        std::string msg = "cannot bind to addr=";
        msg += ip;
        msg += ", port=" + std::to_string(port);
        throw errors::BindError(msg);
    }

    if (::listen(server_sock_fd_.data(), SOMAXCONN) < 0) {
        std::string msg = "cannot start listen on addr=";
        msg += ip;
        msg += ", port=" + std::to_string(port);
        throw errors::ListenError(msg);
    }

    if (port == 0) {
        socklen_t addr_size = sizeof(addr);
        int status = getsockname(server_sock_fd_.data(),
                                 reinterpret_cast<sockaddr *>(&addr),
                                 &addr_size);
        if (status < 0) {
            throw errors::IoServiceError(
                    "cannot get info about self, server_sock_fd="
                    + std::to_string(server_sock_fd_.data()));
        }
        port = ntohs(addr.sin_port);
    }

    src_addr_ = ip;
    src_port_ = port;
}

const std::string &Server::get_src_addr() const noexcept {
    return src_addr_;
}

uint16_t Server::get_src_port() const noexcept {
    return src_port_;
}

trivilog::BaseLogger &Server::get_logger() const noexcept {
    return logger_;
}

const unixprimwrap::Descriptor &Server::get_acceptor_service() const noexcept {
    return server_sock_fd_;
}

bool Server::is_opened() const noexcept {
    return server_sock_fd_.is_valid();;
}

bool Server::is_stopped() const noexcept {
    return is_stopped_;
}

void Server::stop() noexcept {
    is_stopped_ = true;
}

void Server::close() {
    if (is_opened()) {
        if (server_sock_fd_.close() < 0) {
            throw errors::ServerCloseError(
                    "error while closing server socket, server_sock_fd="
                    + std::to_string(server_sock_fd_.data()));
        }
    }
}

void Server::run(Server::EventLoopConfig config, size_t thread_counts) {
    std::vector<std::thread> workers(thread_counts - 1);

    size_t id = 0;

    for (; id < workers.size(); ++id) {
        workers[id] = std::thread([id, this](const  Server::EventLoopConfig &cfg) {
            EpollWorker(id, *this).event_loop(cfg);
        }, std::ref(config));
    }

    EpollWorker(id, *this).event_loop(config);

    for (auto &worker : workers) {
        worker.join();
    }
}

}
