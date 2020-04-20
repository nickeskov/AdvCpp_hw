#include "unixprimwrap/pipe.h"
#include "unixprimwrap/errors.h"

extern "C" {
#include <unistd.h>
}

namespace unixprimwrap {

namespace {

constexpr int PIPE_WRITE_END = 1;
constexpr int PIPE_READ_END = 0;

}

Pipe::Pipe() {
    int pipe_fd[2];

    if (pipe(pipe_fd) != 0) {
        throw errors::PipeCreationError("cannot create pipe");
    }

    read_end_ = Descriptor(pipe_fd[PIPE_READ_END]);
    write_end_ = Descriptor(pipe_fd[PIPE_WRITE_END]);
}

Descriptor &Pipe::get_read_end() noexcept {
    return read_end_;
}

const Descriptor &Pipe::get_read_end() const noexcept {
    return read_end_;
}

Descriptor &Pipe::get_write_end() noexcept {
    return write_end_;
}

const Descriptor &Pipe::get_write_end() const noexcept {
    return write_end_;
}

}
