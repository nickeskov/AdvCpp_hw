#include "process.h"
#include "linuxproc_errors.h"
#include "pipe.h"

#include <algorithm>


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

size_t Process::write(const void *buf, size_t len) {
    if (!fd_process_to_.is_valid()) {
        throw errors::WriteToClosedEndpointError("pipe write endpoint already closed");
    }
    ssize_t bytes_written = ::write(fd_process_to_.data(), buf, len);
    if (bytes_written == -1) {
        throw errors::WriteError("write error occurs while writing to process pipe");
    }
    return bytes_written;
}

void Process::write_exact(const void *buf, size_t len) {
    while (len != 0) {
        size_t bytes_written = write(buf, len);
        len -= bytes_written;
        if (bytes_written == 0 && len != 0) {
            throw errors::WriteError("nothing was written to process pipe");
        }
    }
}

size_t Process::read(void *buf, size_t len) {
    ssize_t bytes_read = 0;
    if (is_readable() && len != 0) {
        bytes_read = ::read(fd_process_from_.data(), buf, len);
        if (bytes_read == 0) {
            is_readable_ = false;
        }
        if (bytes_read == -1) {
            throw errors::ReadError("read error occurs while reading from process pipe");
        }
    }
    return bytes_read;
}

void Process::read_exact(void *buf, size_t len) {
    while (len != 0) {
        if (!is_readable()) {
            throw errors::EofError("EOF reached");
        }
        len -= read(buf, len);
    }
}

bool Process::is_readable() const noexcept {
    return is_readable_;
}

void Process::close_stdin() {
    if (fd_process_to_.is_valid() && fd_process_to_.close() != 0) {
        throw errors::CloseError("close error occurs while closing write pipe for process");
    }
}

Process::~Process() noexcept {
    if (pid_ != -1) {
        kill(pid_, SIGTERM);
        waitpid(pid_, nullptr, 0);
    }
}

void Process::prepare_to_exec(const unixprimwrap::Pipe &pipe_to_child,
                              const unixprimwrap::Pipe &pipe_from_child) {
    auto stdin_dup = unixprimwrap::Descriptor(dup(STDIN_FILENO));
    if (stdin_dup.data() == -1) {
        throw errors::DupError("cannot dup STDIN_FILENO");
    }

    if (pipe_to_child.get_read_end().dup2(STDIN_FILENO) == -1) {
        std::string msg = "cannot dup2 descriptor ";
        msg += std::to_string(pipe_to_child.get_read_end().data()) + "STDIN_FILENO";
        throw errors::DupError(msg);
    }
    if (pipe_from_child.get_write_end().dup2(STDOUT_FILENO) == -1) {
        (void) stdin_dup.dup2(STDIN_FILENO);
        std::string msg = "cannot dup2 descriptor ";
        msg += std::to_string(pipe_from_child.get_write_end().data()) + "STDOUT_FILENO";
        throw errors::DupError(msg);
    }
}

void Process::create_proc(std::string_view path, char *const argv[]) {
    unixprimwrap::Pipe pipe_to_child;
    unixprimwrap::Pipe pipe_from_child;

    pid_ = fork();
    if (pid_ == -1) {
        throw errors::ForkError("cannot fork to create new process");
    }

    if (pid_ == 0) {
        prepare_to_exec(pipe_to_child, pipe_from_child);

        if (::execv(path.data(), argv) == -1) {
            std::string msg = "exec new process fails with process path=";
            throw errors::ExecError(msg.append(path));
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
