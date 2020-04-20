#include "unixprimwrap/fork.h"

#include <sys/wait.h>
#include <unistd.h>
#include <utility>

namespace unixprimwrap {

Fork::Fork() noexcept: pid_(fork()), is_child_(pid_ == 0) {}

Fork::Fork(Fork &&other) noexcept {
    swap(other);
}

Fork &Fork::operator=(Fork &&other) noexcept {
    if (this == &other) {
        return *this;
    }
    kill(SIGTERM);
    wait(nullptr, 0);
    swap(other);
    return *this;
}

void Fork::swap(Fork &other) noexcept {
    std::swap(pid_, other.pid_);
    std::swap(is_child_, other.is_child_);
}

pid_t Fork::data() const noexcept {
    return pid_;
}

bool Fork::is_valid() const noexcept {
    return pid_ >= 0;
}

bool Fork::is_child() const noexcept {
    return is_child_;
}

pid_t Fork::extract() noexcept {
    pid_t pid = pid_;
    pid_ = -1;
    is_child_ = false;
    return pid;
}

// NOLINTNEXTLINE kill method can't be const
int Fork::kill(int sig) {
    int status = -1;
    if (is_valid() && !is_child()) {
        status = ::kill(pid_, sig);
    }
    return status;
}

int Fork::wait(int *wstatus, int options) {
    int status = -1;
    if (is_valid() && !is_child()) {
        status = waitpid(pid_, wstatus, options);
        if (status > 0) {
            pid_ = -1;
            is_child_ = false;
        }
    }
    return status;
}

Fork::~Fork() noexcept {
    if (is_valid() && !is_child()) {
        kill(SIGTERM);
        wait(nullptr, 0);
    }
}

}
