#ifndef PROCESS_H
#define PROCESS_H

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

namespace linuxproc {
constexpr int PIPE_WRITE = 1;
constexpr int PIPE_READ = 0;

class Process {
  public:

    Process(const std::string &path, char *const argv[]);

    Process(const std::string &path, const std::vector<std::string> &argv);

    template<typename ...Args>
    explicit Process(const std::string &path, Args... args);

    Process(const Process &) = delete;

    Process &operator=(const Process &) = delete;

    Process(Process &&process) noexcept;

    Process &operator=(Process &&process) noexcept;

    void swap(Process &rhs) noexcept;

    size_t write(const void *buf, size_t len);

    void write_exact(const void *buf, size_t len);

    size_t read(void *buf, size_t len);

    void read_exact(void *buf, size_t len);

    [[nodiscard]] bool is_readable() const noexcept;

    void close_stdin();

    [[nodiscard]] pid_t get_pid() const noexcept;

    ~Process() noexcept;

  private:
    pid_t pid_;
    int fd_process_to_;
    int fd_process_from_;

    Process() noexcept;

    template<typename ...Args>
    std::string close_an_error(Args... fds) const {
        std::stringstream errors;
        (..., (close(fds) == -1 ? errors << std::strerror(errno) << std::endl : errors));
        return errors.str();
    }

    void create_proc_pipes(int pipe_to_child[2], int pipe_from_child[2]);

    void prepare_to_exec(int pipe_to_child[2], int pipe_from_child[2]);

    void parent_process_cleanups(int pipe_to_child[2], int pipe_from_child[2]);

    void create_proc(const std::string &path, char *const argv[]);
};

template<typename... Args>
Process::Process(const std::string &path, Args... args)
        : pid_(-1), fd_process_to_(-1), fd_process_from_(-1) {

    int pipe_to_child[2];
    int pipe_from_child[2];

    create_proc_pipes(pipe_to_child, pipe_from_child);

    pid_ = fork();
    if (pid_ == -1) {
        throw std::runtime_error(std::strerror(errno));
    }

    if (pid_ == 0) {
        prepare_to_exec(pipe_to_child, pipe_from_child);

        if (::execl(path.data(), args.data()..., nullptr) == -1) {
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

}


#endif //PROCESS_H
