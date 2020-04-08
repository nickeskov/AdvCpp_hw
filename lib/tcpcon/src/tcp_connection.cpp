#include "tcp_connection.h"
#include "tcpcon_errors.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utility>
#include <string>

namespace tcpcon::ipv4 {

Connection::Connection(std::string_view ip, uint16_t port)
        : sock_fd_(socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) {

    if (!sock_fd_.is_valid()) {
        throw errors::SocketError("cannot create IPV4 socket");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(addr.sin_family, ip.data(), &addr.sin_addr) == 0) {
        std::string msg = "invalid ip address, ip=";
        msg.append(ip);
        throw errors::InvalidAddressError(msg);
    }

    if (::connect(sock_fd_.data(), reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        std::string msg = "cannot connect to, addr=";
        msg.append(ip) += ", port=" + std::to_string(port);
        throw errors::ConnOpenError(msg);
    }

    dst_addr_ = ip;
    dst_port_ = port;

    set_src_endpoint();
}

size_t Connection::write(const void *buf, size_t len) {
    if (!is_opened()) {
        throw errors::ClosedEndpointError("write to closed endpoint, sock_fd="
                                          + std::to_string(sock_fd_.data()));
    }
    ssize_t bytes_written = ::send(sock_fd_.data(), buf, len, MSG_NOSIGNAL);
    if (bytes_written == -1) {
        throw errors::WriteError(
                "write error occurs while writing to endpoint, sock_fd="
                + std::to_string(sock_fd_.data()));
    }
    return bytes_written;
}

void Connection::write_exact(const void *buf, size_t len) {
    while (len != 0) {
        size_t bytes_written = write(buf, len);
        len -= bytes_written;
        if (bytes_written == 0 && len != 0) {
            throw errors::WriteError("nothing was written to endpoint, sock_fd="
                                     + std::to_string(sock_fd_.data()));
        }
    }
}

size_t Connection::read(void *buf, size_t len) {
    if (!is_opened()) {
        throw errors::ClosedEndpointError("write to closed endpoint, sock_fd="
                                          + std::to_string(sock_fd_.data()));
    }

    ssize_t bytes_read = 0;
    if (is_opened() && len != 0) {
        bytes_read = ::recv(sock_fd_.data(), buf, len, MSG_NOSIGNAL);
        if (bytes_read == 0) {
            is_readable_ = false;
        }
        if (bytes_read == -1) {
            throw errors::WriteError(
                    "write error occurs while reading from endpoint, sock_fd="
                    + std::to_string(sock_fd_.data()));
        }
    }
    return bytes_read;
}

void Connection::read_exact(void *buf, size_t len) {
    while (len != 0) {
        if (!is_readable()) {
            throw errors::EofError("EOF reached, sock_fd=" + std::to_string(sock_fd_.data()));
        }
        len -= read(buf, len);
    }
}

void Connection::connect(std::string_view ip, uint16_t port) {
    *this = Connection(ip, port);
}

void Connection::set_read_timeout(int seconds) {
    timeval timeout{seconds, 0};
    if (setsockopt(sock_fd_.data(), SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        throw errors::ConnectionError(
                "cannot set read timeout, sock_fd="
                + std::to_string(sock_fd_.data()) + ", seconds=" + std::to_string(seconds));
    }
}

void Connection::set_write_timeout(int seconds) {
    timeval timeout{seconds, 0};
    if (setsockopt(sock_fd_.data(), SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        throw errors::ConnectionError(
                "cannot set write timeout, sock_fd="
                + std::to_string(sock_fd_.data()) + ", seconds=" + std::to_string(seconds));
    }
}

bool Connection::is_opened() const noexcept {
    return sock_fd_.is_valid();
}

bool Connection::is_readable() const noexcept {
    return is_readable_;
}

void Connection::close() {
    if (is_opened()) {
        int sock_fd = sock_fd_.data();
        int status = ::shutdown(sock_fd, SHUT_RDWR);
        if (sock_fd_.close() < 0) {
            status = -1;
        }
        if (status < 0) {
            throw errors::ConnCloseError(
                    "error while closing socket, sock_fd=" + std::to_string(sock_fd));
        }
    }
}

Connection::~Connection() noexcept {
    if (is_opened()) {
        ::shutdown(sock_fd_.data(), SHUT_RDWR);
        sock_fd_.close();
    }
}

Connection::Connection(linuxproc::Descriptor &&endpoint, std::string &&dst_addr, uint16_t dst_port)
        : sock_fd_(std::move(endpoint)), dst_addr_(std::move(dst_addr)), dst_port_(dst_port) {
    set_src_endpoint();
}

void Connection::set_src_endpoint() {
    sockaddr_in addr{};
    socklen_t addr_size = sizeof(addr);
    if (getsockname(sock_fd_.data(), reinterpret_cast<sockaddr *>(&addr), &addr_size) < 0) {
        throw errors::SocketError(
                "cannot get info about self endpoint, sock_fd="
                + std::to_string(sock_fd_.data()));
    }

    char buff[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, buff, INET_ADDRSTRLEN);

    src_addr_ = buff;
    src_port_ = ntohs(addr.sin_port);
}

const std::string &Connection::get_dst_addr() const noexcept {
    return dst_addr_;
}

const std::string &Connection::get_src_addr() const noexcept {
    return src_addr_;
}

uint16_t Connection::get_dst_port() const noexcept {
    return dst_port_;
}

uint16_t Connection::get_src_port() const noexcept {
    return src_port_;
}

std::string Connection::to_string() const {
    return R"({"dst_addr"=")" + get_dst_addr()
           + R"(","dst_port"=)" + std::to_string(get_dst_port())
           + R"(,"src_addr"=")" + get_src_addr()
           + R"(","src_port"=)" + std::to_string(get_src_port()) + "}";
}

}

