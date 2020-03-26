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
constexpr inline bool can_be_string_view_v = std::is_convertible_v<pack_t<Args...>, std::string_view>;

class Process {
  public:

    template<typename ...Args, typename = void>
    explicit Process(std::string_view path, Args &&... args);

    Process(std::string_view path, char *const argv[]);

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

    static void prepare_to_exec(const Pipe &pipe_to_child, const Pipe &pipe_from_child);

    void create_proc(std::string_view path, char *const argv[]);
};

template<typename ...Args>
using pack_t = typename std::common_type<typename std::decay<Args>::type ...>::type;

template<typename ...Args,
        typename = std::enable_if_t<can_be_string_view_v<Args...>, std::string_view>>
Process::Process(std::string_view path, Args &&... args) {
    std::initializer_list<std::string_view> args_views = {
            std::forward<Args>(args)...,
            nullptr
    };
    // NOLINTNEXTLINE: Redundant initialization
    std::array<const char *, sizeof...(Args) + 1> args_cstrs;
    std::transform(args_views.begin(), args_views.end(), args_cstrs.begin(), [](const auto &str_view) {
        return str_view.data();
    });
    create_proc(path, const_cast<char **const>(args_cstrs.data()));
}

}


#endif //HW_PROCESS_H
