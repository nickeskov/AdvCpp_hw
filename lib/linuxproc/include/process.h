#ifndef HW_PROCESS_H
#define HW_PROCESS_H

#include <string>
#include <vector>
#include <string_view>
#include <cstddef>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <csignal>

#include <wait.h>
#include <unistd.h>
#include <utility>

#include "descriptor.h"
#include "pipe.h"
#include "errors.h"

namespace linuxproc {
constexpr int PIPE_WRITE = 1;
constexpr int PIPE_READ = 0;

class Process {
  public:

    Process(const std::string &path, char *const argv[]);

    template<typename ...Args>
    explicit Process(const std::string &path, Args &&... args);

    Process(const Process &) = delete;

    Process &operator=(const Process &) = delete;

    Process(Process &&process) noexcept;

    Process &operator=(Process &&process) noexcept;

    void swap(Process &rhs) noexcept;

    ssize_t write(const void *buf, size_t len);

    void write_exact(const void *buf, size_t len);

    ssize_t read(void *buf, size_t len);

    void read_exact(void *buf, size_t len);

    [[nodiscard]] bool is_readable() const noexcept;

    void close_stdin();

    [[nodiscard]] pid_t get_pid() const noexcept;

    ~Process() noexcept;

  private:
    pid_t pid_ = -1;
    Descriptor fd_process_to_;
    Descriptor fd_process_from_;

    Process() noexcept = default;

//    template<typename ...ArgsT>
//    std::string close_an_error(ArgsT&&... fds) const {
//        std::stringstream errors;
//        (..., (close(std::forward<ArgsT>(fds)) == -1 ? errors << std::strerror(errno) << std::endl : errors));
//        return errors.str();
//    }

    static void prepare_to_exec(const Pipe &pipe_to_child, const Pipe &pipe_from_child);

    void create_proc(const std::string &path, char *const argv[]);
};

template<typename... ArgsT>
Process::Process(const std::string &path, ArgsT &&... args) {
    Pipe pipe_to_child;
    Pipe pipe_from_child;

    pid_ = fork();
    if (pid_ == -1) {
        throw ForkError();
    }

    if (pid_ == 0) {
        prepare_to_exec(pipe_to_child, pipe_from_child);
        if (::execl(path.data(), std::forward<ArgsT>(args).data()..., nullptr) == -1) {
            throw ExecError();
        }
    } else {
        fd_process_to_ = std::move(pipe_to_child.get_write_end());
        fd_process_from_ = std::move(pipe_from_child.get_read_end());
    }
}

}


#endif //HW_PROCESS_H
