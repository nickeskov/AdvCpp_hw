#ifndef LINUXPROC_LINUXPROC_PROCESS_H
#define LINUXPROC_LINUXPROC_PROCESS_H

#include <cstddef>
#include <csignal>
#include <array>
#include <string_view>
#include <algorithm>
#include <utility>

#include "unixprimwrap/descriptor.h"
#include "unixprimwrap/pipe.h"
#include "unixprimwrap/errors.h"

namespace linuxproc {

class Process {
  public:
    template<typename ...Args,
            typename = std::enable_if_t<std::is_constructible_v<std::string_view, Args...>>>
    explicit Process(std::string_view path, Args &&... args) {
        char *const arguments[]{
                const_cast<char *>(std::string_view{args}.data())...,
                nullptr
        };
        create_proc(path, arguments);
    }

    Process(std::string_view path, char *const argv[]);

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
    pid_t pid_ = -1;
    bool is_readable_ = true;
    unixprimwrap::Descriptor fd_process_to_;
    unixprimwrap::Descriptor fd_process_from_;

    Process() noexcept = default;

    static void prepare_to_exec(const unixprimwrap::Pipe &pipe_to_child,
                                const unixprimwrap::Pipe &pipe_from_child);

    void create_proc(std::string_view path, char *const argv[]);
};

}

#endif //LINUXPROC_LINUXPROC_PROCESS_H
