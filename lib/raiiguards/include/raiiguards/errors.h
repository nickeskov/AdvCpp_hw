#ifndef RAIIGUARDS_RAIIGUARDS_ERRORS_H
#define RAIIGUARDS_RAIIGUARDS_ERRORS_H

#include <stdexcept>
#include <string_view>

namespace raiiguards::errors {

class RuntimeError : public std::runtime_error {
  public:
    explicit RuntimeError(std::string_view what_arg);

    [[nodiscard]] int errno_code() const noexcept;

    ~RuntimeError() override = default;

  private:
    int errno_code_ = errno;
};

}

#endif //RAIIGUARDS_RAIIGUARDS_ERRORS_H
