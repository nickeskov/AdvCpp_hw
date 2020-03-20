#include "process.h"

#include <sstream>
#include <algorithm>

namespace linuxproc {
namespace {

constexpr int DESCRIPTOR_ALREADY_CLOSED = -2;

}


Process::Process(const std::string &path, char *const argv[])
        : pid_(-1), fd_process_to_(-1), fd_process_from_(-1) {
    create_proc(path, argv);
}

Process::Process(const std::string &path, const std::vector<std::string> &argv)
        : pid_(-1), fd_process_to_(-1), fd_process_from_(-1) {
    std::vector<const char *> args;
    args.reserve(argv.size() + 1);

    std::transform(argv.cbegin(), argv.cend(), args.begin(),
                   [](auto arg) {
                       return arg.data();
                   });
    args.push_back(nullptr);

    create_proc(path, const_cast<char *const *>(args.data()));
}

size_t Process::write(const void *buf, size_t len) {
    return ::write(fd_process_to_, buf, len);
}

void Process::write_exact(const void *buf, size_t len) {
    while (len != 0) {
        int bytes_written = ::write(fd_process_to_, buf, len);
        if (bytes_written == -1) {
            std::stringstream errors;
            errors << strerror(errno) << std::endl;
            throw std::runtime_error(errors.str());
        }
        len -= bytes_written;
        if (bytes_written == 0 && len != 0) {
            throw std::runtime_error("cant write, zero bytes written but buf len is not zero");
        }
    }
}

size_t Process::read(void *buf, size_t len) {
    return ::read(fd_process_from_, buf, len);
}

void Process::read_exact(void *buf, size_t len) {
    size_t offset = 0;
    while (offset < len) {
        int bytes_read = ::read(fd_process_from_, buf, len);
        if (bytes_read == -1) {
            std::stringstream errors;
            errors << strerror(errno) << std::endl;
            throw std::runtime_error(errors.str());
        }
        offset += bytes_read;
        if (bytes_read == 0 && len != offset) {
            throw std::runtime_error("cant read, zero bytes read but buf len is not zero");
        }
    }
}

bool Process::is_readable() const noexcept {
    char tmp;
    return ::read(fd_process_from_, &tmp, 0) == 0;
}

void Process::close_stdin() {
    if (fd_process_to_ != DESCRIPTOR_ALREADY_CLOSED
        && close(fd_process_to_) == -1) {

        std::stringstream errors;
        errors << strerror(errno) << std::endl;
        throw std::runtime_error(errors.str());
    }
    fd_process_to_ = DESCRIPTOR_ALREADY_CLOSED;
}

Process::~Process() noexcept {
    if (fd_process_to_ != DESCRIPTOR_ALREADY_CLOSED && close(fd_process_to_) == -1) {
        std::cerr << "ERROR:" << "process " << pid_ << ":"
                  << strerror(errno) << std::endl;
    }
    if (close(fd_process_from_) == -1) {
        std::cerr << "ERROR:" << "process " << pid_ << ":"
                  << strerror(errno) << std::endl;
    }
    if (kill(pid_, SIGTERM) == -1) {
        std::cerr << "ERROR:" << "process " << pid_ << ":"
                  << strerror(errno) << std::endl;
    }
    int status;
    if (waitpid(pid_, &status, 0) == -1) {
        std::cerr << "ERROR:" << "process " << pid_ << ":"
                  << strerror(errno) << std::endl;
    } else if (status != 0) {
        std::cerr << "" << "process " << pid_
                  << " was exited with code " << status << std::endl;
    }
}

void Process::create_proc_pipes(int pipe_to_child[2], int pipe_from_child[2]) {
    if (pipe(pipe_to_child) == -1) {
        throw std::runtime_error(std::strerror(errno));
    }

    if (pipe(pipe_from_child) == -1) {
        std::stringstream errors;
        errors << std::strerror(errno) << std::endl
               << close_an_error(
                       pipe_to_child[PIPE_WRITE],
                       pipe_to_child[PIPE_READ]);
        throw std::runtime_error(errors.str());
    }
}

void Process::prepare_to_exec(int pipe_to_child[2], int pipe_from_child[2]) {
    if (close(pipe_to_child[PIPE_WRITE]) == -1) {
        std::stringstream errors;
        errors << std::strerror(errno) << std::endl
               << close_an_error(
                       pipe_to_child[PIPE_READ],
                       pipe_from_child[PIPE_READ],
                       pipe_from_child[PIPE_WRITE]);
        throw std::runtime_error(errors.str());
    }
    if (close(pipe_from_child[PIPE_READ]) == -1) {
        std::stringstream errors;
        errors << std::strerror(errno) << std::endl
               << close_an_error(
                       pipe_to_child[PIPE_READ],
                       pipe_from_child[PIPE_WRITE]);
        throw std::runtime_error(errors.str());
    }

    if (dup2(pipe_to_child[PIPE_READ], STDIN_FILENO) == -1
        || dup2(pipe_from_child[PIPE_WRITE], STDOUT_FILENO) == -1) {

        std::stringstream errors;
        errors << std::strerror(errno) << std::endl
               << close_an_error(
                       pipe_to_child[PIPE_READ],
                       pipe_from_child[PIPE_WRITE]);
        throw std::runtime_error(errors.str());
    }

    if (close(pipe_to_child[PIPE_READ]) == -1) {
        std::stringstream errors;
        errors << std::strerror(errno) << std::endl
               << close_an_error(pipe_from_child[PIPE_WRITE]);
        std::cerr << errors.str();
        throw std::runtime_error(errors.str());
    }
    if (close(pipe_from_child[PIPE_WRITE]) == -1) {
        std::stringstream errors;
        errors << std::strerror(errno) << std::endl;
        throw std::runtime_error(errors.str());
    }
}

void Process::parent_process_cleanups(int pipe_to_child[2], int pipe_from_child[2]) {
    if (close(pipe_to_child[PIPE_READ]) == -1) {
        std::stringstream errors;
        errors << std::strerror(errno) << std::endl
               << close_an_error(
                       pipe_to_child[PIPE_WRITE],
                       pipe_from_child[PIPE_READ],
                       pipe_from_child[PIPE_WRITE]
               );
        if (kill(pid_, SIGTERM) == -1) {
            errors << std::strerror(errno) << std::endl;
            throw std::runtime_error(errors.str());
        }
        if (waitpid(pid_, nullptr, 0) == -1) {
            errors << std::strerror(errno) << std::endl;
        }
        throw std::runtime_error(errors.str());
    }
    if (close(pipe_from_child[PIPE_WRITE]) == -1) {
        std::stringstream errors;
        errors << std::strerror(errno) << std::endl
               << close_an_error(
                       pipe_to_child[PIPE_WRITE],
                       pipe_from_child[PIPE_READ]
               );
        if (kill(pid_, SIGTERM) == -1) {
            errors << std::strerror(errno) << std::endl;
            throw std::runtime_error(errors.str());
        }
        if (waitpid(pid_, nullptr, 0) == -1) {
            errors << std::strerror(errno) << std::endl;
        }
        throw std::runtime_error(errors.str());
    }
}


void Process::create_proc(const std::string &path, char *const argv[]) {
    int pipe_to_child[2];
    int pipe_from_child[2];

    create_proc_pipes(pipe_to_child, pipe_from_child);

    pid_ = fork();
    if (pid_ == -1) {
        throw std::runtime_error(std::strerror(errno));
    }

    if (pid_ == 0) {
        prepare_to_exec(pipe_to_child, pipe_from_child);

        if (::execv(path.data(), argv) == -1) {
            std::stringstream errors;
            errors << std::strerror(errno) << std::endl;
            throw std::runtime_error(errors.str());
        }
    } else {
        parent_process_cleanups(pipe_to_child, pipe_from_child);

        fd_process_to_ = pipe_to_child[PIPE_WRITE];
        fd_process_from_ = pipe_from_child[PIPE_READ];
    }
}

pid_t Process::get_pid() const noexcept {
    return pid_;
}

}