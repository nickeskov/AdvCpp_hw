#include "process.h"
#include "errors.h"

#include <sstream>
#include <algorithm>
#include <pipe.h>

namespace linuxproc {

Process::Process(std::string_view path, char *const argv[]) {
    create_proc(path, argv);
}

Process::Process(Process &&process) noexcept {
    swap(process);
}

Process &Process::operator=(Process &&process) noexcept {
    if (this == &process) {
        return *this;
    }
    Process().swap(*this);
    swap(process);
    return *this;
}

void Process::swap(Process &rhs) noexcept {
    std::swap(pid_, rhs.pid_);
    std::swap(fd_process_to_, rhs.fd_process_to_);
    std::swap(fd_process_from_, rhs.fd_process_from_);
}

ssize_t Process::write(const void *buf, size_t len) {
    return ::write(fd_process_to_.data(), buf, len);
}

void Process::write_exact(const void *buf, size_t len) {
    while (len != 0) {
        int bytes_written = write(buf, len);
        if (bytes_written == -1) {
            throw errors::WriteError();
        }
        len -= bytes_written;
        if (bytes_written == 0 && len != 0) {
            throw errors::WriteError();
        }
    }
}

ssize_t Process::read(void *buf, size_t len) {
    return ::read(fd_process_from_.data(), buf, len);
}

void Process::read_exact(void *buf, size_t len) {
    size_t offset = 0;
    while (offset < len) {
        int bytes_read = read(buf, len);
        if (bytes_read == -1) {
            throw errors::ReadError();
        }
        offset += bytes_read;
        if (bytes_read == 0 && len != offset) {
            throw errors::ReadError();
        }
    }
}

bool Process::is_readable() const noexcept {
    char tmp;
    return ::read(fd_process_from_.data(), &tmp, 0) == 0;
}

void Process::close_stdin() {
    fd_process_to_ = Descriptor();
}

Process::~Process() noexcept {
    if (pid_ != -1) {
        kill(pid_, SIGTERM);
        waitpid(pid_, nullptr, 0);
    }
}

void Process::prepare_to_exec(const Pipe &pipe_to_child, const Pipe &pipe_from_child) {
    auto stdin_dup = Descriptor(dup(STDIN_FILENO));
    if (stdin_dup.data() == -1) {
        throw errors::DupError();
    }

    if (pipe_to_child.get_read_end().dup2(STDIN_FILENO) == -1) {
        throw errors::DupError();
    }
    if (pipe_from_child.get_write_end().dup2(STDOUT_FILENO) == -1) {
        (void) stdin_dup.dup2(STDIN_FILENO);
        throw errors::DupError();
    }
}

void Process::create_proc(std::string_view path, char *const argv[]) {
    Pipe pipe_to_child;
    Pipe pipe_from_child;

    pid_ = fork();
    if (pid_ == -1) {
        throw errors::ForkError();
    }

    if (pid_ == 0) {
        prepare_to_exec(pipe_to_child, pipe_from_child);

        if (::execv(path.data(), argv) == -1) {
            throw errors::ExecError();
        }
    } else {
        fd_process_to_ = std::move(pipe_to_child.get_write_end());
        fd_process_from_ = std::move(pipe_from_child.get_read_end());
    }
}

pid_t Process::get_pid() const noexcept {
    return pid_;
}

}
