#ifndef HW_PROCESS_H
#define HW_PROCESS_H

#include <cstddef>
#include <csignal>
#include <array>
#include <string_view>
#include <algorithm>
#include <utility>

#include <wait.h>
#include <unistd.h>

#include "descriptor.h"
#include "pipe.h"
#include "errors.h"

namespace linuxproc {

template<typename ...Args>
using pack_t = typename std::common_type<typename std::decay<Args>::type ...>::type;

template<typename ...Args>
constexpr inline bool can_construct_string_view_v = std::is_constructible_v<std::string_view, Args...>;

class Process {
  public:
    template<typename ...Args,
            typename = std::enable_if_t<can_construct_string_view_v<pack_t<Args...>>>>
    explicit Process(const std::string_view &path, Args &&... args) {
        char *const arguments[] = {
                const_cast<char *>(std::string_view(std::forward<Args>(args)).data())...,
                nullptr
        };
        create_proc(path, arguments);
    }

    Process(const std::string_view &path, char *const argv[]);

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
    Descriptor fd_process_to_;
    Descriptor fd_process_from_;

    Process() noexcept = default;

    static void prepare_to_exec(const Pipe &pipe_to_child, const Pipe &pipe_from_child);

    void create_proc(const std::string_view &path, char *const argv[]);
};

}

#endif //HW_PROCESS_H
