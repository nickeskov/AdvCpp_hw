#ifndef UNIXPRIMWRAP_UNIXPRIMWRAP_FORK_H
#define UNIXPRIMWRAP_UNIXPRIMWRAP_FORK_H

extern "C" {
#include <sys/types.h>
}

namespace unixprimwrap {

class Fork {
  public:
    Fork() noexcept;

    Fork(const Fork &other) = delete;

    Fork &operator=(const Fork &other) = delete;

    Fork(Fork &&other) noexcept;

    Fork &operator=(Fork &&other) noexcept;

    void swap(Fork &other) noexcept;

    [[nodiscard]] pid_t data() const noexcept;

    [[nodiscard]] bool is_valid() const noexcept;

    [[nodiscard]] bool is_child() const noexcept;

    [[nodiscard]] pid_t extract() noexcept;

    int kill(int sig);

    int wait(int *wstatus, int options);

    ~Fork() noexcept;

  private:
    pid_t pid_ = -1;
    bool is_child_ = false;
};

}

#endif //UNIXPRIMWRAP_UNIXPRIMWRAP_PROCESS_H
