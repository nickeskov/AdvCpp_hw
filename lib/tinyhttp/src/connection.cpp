#include "tinyhttp/connection.h"
#include "tinyhttp/errors.h"
#include "tinyhttp/utils.h"

#include <string>

extern "C" {
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}

namespace tinyhttp {

Connection::Connection(std::string_view ip, uint16_t port, bool set_nonblock) {
    int sock_type = SOCK_STREAM;
    if (set_nonblock) {
        // NOLINTNEXTLINE is valid values and this constants is unsigned
        sock_type |= SOCK_NONBLOCK;
    }

    sock_fd_ = unixprimwrap::Descriptor{
            socket(PF_INET, sock_type, IPPROTO_TCP)
    };

    if (!sock_fd_.is_valid()) {
        throw errors::IoServiceError("cannot create IPV4 socket");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(addr.sin_family, ip.data(), &addr.sin_addr) == 0) {
        std::string msg = "invalid ip address, ip=";
        msg += ip;
        throw errors::InvalidAddressError(msg);
    }

    int status = ::connect(sock_fd_.data(),
                           reinterpret_cast<sockaddr *>(&addr),
                           sizeof(addr));

    if (status < 0 && errno != EINPROGRESS) {
        std::string msg = "cannot connect to, addr=";
        msg.append(ip) += ", port=" + std::to_string(port);
        throw errors::ConnOpenError(msg);
    }

    dst_addr_ = ip;
    dst_port_ = port;

    set_src_endpoint();
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

const unixprimwrap::Descriptor &Connection::get_io_service() const noexcept {
    return sock_fd_;
}

void Connection::set_nonblock(bool opt) {
    if (!is_opened()) {
        throw errors::IoServiceError("set_nonblock to closed endpoint, sock_fd="
                                     + std::to_string(sock_fd_.data()));
    }

    if (utils::set_nonblock(sock_fd_.data(), opt) < 0) {
        throw errors::IoServiceError("cannot set nonblock for socket, sock_fd="
                                     + std::to_string(sock_fd_.data()));
    }
}

std::string Connection::to_string() const {
    return R"({"dst_addr"=")" + get_dst_addr()
           + R"(","dst_port"=)" + std::to_string(get_dst_port())
           + R"(,"src_addr"=")" + get_src_addr()
           + R"(","src_port"=)" + std::to_string(get_src_port()) + "}";
}

void Connection::connect(std::string_view ip, uint16_t port) {
    *this = Connection(ip, port);
}

void Connection::set_read_timeout(int seconds) {
    timeval timeout{};
    timeout.tv_sec = seconds;

    int status = setsockopt(sock_fd_.data(),
                            SOL_SOCKET,
                            SO_RCVTIMEO,
                            &timeout,
                            sizeof(timeout));

    if (status < 0) {
        throw errors::TimeoutSetError(
                "cannot set read timeout, sock_fd="
                + std::to_string(sock_fd_.data()) + ", seconds=" + std::to_string(seconds));
    }
}

void Connection::set_write_timeout(int seconds) {
    timeval timeout{};
    timeout.tv_sec = seconds;

    int status = setsockopt(sock_fd_.data(),
                            SOL_SOCKET,
                            SO_SNDTIMEO,
                            &timeout,
                            sizeof(timeout));

    if (status < 0) {
        throw errors::TimeoutSetError(
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

void Connection::set_src_endpoint() {
    sockaddr_in addr{};
    socklen_t addr_size = sizeof(addr);

    int status = getsockname(sock_fd_.data(),
                             reinterpret_cast<sockaddr *>(&addr),
                             &addr_size);
    if (status < 0) {
        throw errors::IoServiceError(
                "cannot get info about self endpoint, sock_fd="
                + std::to_string(sock_fd_.data()));
    }

    char buff[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, buff, INET_ADDRSTRLEN);

    src_addr_ = buff;
    src_port_ = ntohs(addr.sin_port);
}

ssize_t Connection::write(const void *buf, size_t len) {
    if (!is_opened()) {
        throw errors::ClosedEndpointError("write to closed endpoint, sock_fd="
                                          + std::to_string(sock_fd_.data()));
    }

    ssize_t bytes_written = ::send(sock_fd_.data(), buf, len, MSG_NOSIGNAL);

    if (bytes_written == -1
        && errno != EAGAIN
        && errno != EWOULDBLOCK) {

        throw errors::WriteError(
                "write error occurs while writing to endpoint, sock_fd="
                + std::to_string(sock_fd_.data()));
    }
    return bytes_written;
}

ssize_t Connection::read(void *buf, size_t len) {
    if (!is_opened()) {
        throw errors::ClosedEndpointError("write to closed endpoint, sock_fd="
                                          + std::to_string(sock_fd_.data()));
    }

    ssize_t bytes_read = 0;
    if (len != 0) {
        bytes_read = ::recv(sock_fd_.data(), buf, len, MSG_NOSIGNAL);
        if (bytes_read == 0) {
            is_readable_ = false;
        }
        if (bytes_read == -1
            && errno != EAGAIN
            && errno != EWOULDBLOCK) {

            throw errors::ReadError(
                    "write error occurs while reading from endpoint, sock_fd="
                    + std::to_string(sock_fd_.data()));
        }
    }
    return bytes_read;
}

std::string &Connection::get_io_buffer() noexcept {
    return io_buffer_;
}

const std::string &Connection::get_io_buffer() const noexcept {
    return io_buffer_;
}

ssize_t Connection::read_in_io_buff(size_t len) {
    tmp_read_buffer_.clear();
    tmp_read_buffer_.resize(len);

    ssize_t bytes_read = read(tmp_read_buffer_.data(), len);
    if (bytes_read <= 0) {
        return bytes_read;
    }

    tmp_read_buffer_.resize(bytes_read);
    io_buffer_ += tmp_read_buffer_;

    return bytes_read;
}

ssize_t Connection::write_from_io_buff(size_t len) {
    if (io_buffer_.size() < len) {
        len = io_buffer_.size();
    }
    ssize_t bytes_written = write(io_buffer_.data(), len);
    if (bytes_written <= 0) {
        return bytes_written;
    }

    io_buffer_.erase(0, len);
    return bytes_written;
}

Connection::Connection(unixprimwrap::Descriptor &&endpoint, std::string &&dst_addr, uint16_t dst_port)
        : sock_fd_(std::move(endpoint)), dst_addr_(std::move(dst_addr)), dst_port_(dst_port) {
    set_src_endpoint();
}

}
