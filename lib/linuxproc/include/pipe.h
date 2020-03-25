#ifndef HW_PIPE_H
#define HW_PIPE_H

#include "descriptor.h"

namespace linuxproc {

class Pipe {
  public:
    Pipe();

    [[nodiscard]] Descriptor &get_read_end() noexcept;

    [[nodiscard]] const Descriptor &get_read_end() const noexcept;

    [[nodiscard]] Descriptor &get_write_end() noexcept;

    [[nodiscard]] const Descriptor &get_write_end() const noexcept;

  private:
    Descriptor read_end_;
    Descriptor write_end_;
};

}

#endif //HW_PIPE_H
