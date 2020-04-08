#include "tcp_server.h"
#include "tcpcon_errors.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utility>

namespace tcpcon::ipv4 {

Server::Server(std::string_view ip, uint16_t port)
        : server_sock_fd_(socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) {

    if (!server_sock_fd_.is_valid()) {
        throw errors::SocketError("cannot create IPV4 socket");
    }

    int yes = 1;
    if (setsockopt(server_sock_fd_.data(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        throw errors::SocketError("cannot set SO_REUSEADDR to IPV4 socket");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(addr.sin_family, ip.data(), &addr.sin_addr) == 0) {
        std::string msg = "invalid ip address, ip=";
        msg.append(ip);
        throw errors::InvalidAddressError(msg);
    }

    if (::bind(server_sock_fd_.data(), reinterpret_cast<sockaddr *>(&addr),
               sizeof(addr)) < 0) {
        std::string msg = "cannot bind to addr=";
        msg.append(ip) += ", port=" + std::to_string(port);
        throw errors::BindError(msg);
    }

    if (::listen(server_sock_fd_.data(), SOMAXCONN) < 0) {
        std::string msg = "cannot start listen on addr=";
        msg.append(ip) += ", port=" + std::to_string(port);
        throw errors::ListenError(msg);
    }

    if (port == 0) {
        socklen_t addr_size = sizeof(addr);
        if (getsockname(server_sock_fd_.data(), reinterpret_cast<sockaddr *>(&addr), &addr_size) < 0) {
            throw errors::SocketError(
                    "cannot get info about self, server_sock_fd="
                    + std::to_string(server_sock_fd_.data()));
        }
        port = ntohs(addr.sin_port);
    }

    src_addr = ip;
    src_port = port;
}

const std::string &Server::get_src_addr() const noexcept {
    return src_addr;
}

uint32_t Server::get_src_port() const noexcept {
    return src_port;
}

Connection Server::accept() {
    sockaddr_in client_addr{};
    socklen_t addr_size = sizeof(client_addr);

    auto client_fd = linuxproc::Descriptor(
            ::accept(server_sock_fd_.data(), reinterpret_cast<sockaddr *>(&client_addr), &addr_size));

    if (!client_fd.is_valid()) {
        throw errors::AcceptError("cannot accept new connection, server_sock_fd="
                                  + std::to_string(server_sock_fd_.data()));
    }

    char buff[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, buff, INET_ADDRSTRLEN);

    std::string dst_addr = buff;
    uint32_t dst_port = ntohs(client_addr.sin_port);

    return Connection(std::move(client_fd), std::move(dst_addr), dst_port);
}

bool Server::is_opened() const noexcept {
    return server_sock_fd_.is_valid();
}

void Server::close() {
    if (is_opened()) {
        if (server_sock_fd_.close() < 0) {
            throw errors::ConnCloseError(
                    "error while closing server socket, server_sock_fd="
                    + std::to_string(server_sock_fd_.data()));
        }
    }
}

}
