#include "descriptor.h"
#include "errors.h"

#include <unistd.h>

namespace linuxproc {

Descriptor::Descriptor(int &fd) noexcept: fd_(fd) {
    fd = -1;
}

Descriptor::Descriptor(int &&fd) noexcept: Descriptor(fd) {}

Descriptor::Descriptor(const Descriptor &other) {
    if (other.is_valid()) {
        fd_ = dup(other.fd_);
        if (!is_valid()) {
            std::string msg = "cannot dup descriptor with value ";
            msg += std::to_string(other.fd_);
            throw errors::DupError(msg);
        }
    } else {
        fd_ = other.fd_;
    }
}

Descriptor &Descriptor::operator=(const Descriptor &other) {
    if (this == &other) {
        return *this;
    }
    Descriptor(other).swap(*this);
    return *this;
}

Descriptor::Descriptor(Descriptor &&other) noexcept {
    swap(other);
}

Descriptor &Descriptor::operator=(Descriptor &&other) noexcept {
    if (this == &other) {
        return *this;
    }
    Descriptor().swap(*this);
    swap(other);
    return *this;
}

void Descriptor::swap(Descriptor &other) noexcept {
    std::swap(fd_, other.fd_);
}

int Descriptor::data() const noexcept {
    return fd_;
}

Descriptor::operator int() const noexcept {
    return fd_;
}

Descriptor::~Descriptor() noexcept {
    close();
}

int Descriptor::close() noexcept {
    int status = -1;
    if (is_valid()) {
        status = ::close(fd_);
        fd_ = -1;
    }
    return status;
}

bool Descriptor::is_valid() const noexcept {
    return fd_ > -1;
}

int Descriptor::dup2(int fd) const noexcept {
    return ::dup2(fd_, fd);
}

}
