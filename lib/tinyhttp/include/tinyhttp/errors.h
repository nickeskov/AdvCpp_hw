#ifndef TINYHTTP_TINYHTTP_ERRORS_H
#define TINYHTTP_TINYHTTP_ERRORS_H

#include <exception>
#include <stdexcept>
#include <string_view>
#include <cerrno>

namespace tinyhttp::errors {

class RuntimeError : public std::runtime_error {
  public:
    explicit RuntimeError(std::string_view what_arg);

    [[nodiscard]] int errno_code() const noexcept;

    ~RuntimeError() override = default;

  private:
    int errno_code_ = errno;
};

class HttpBaseError : public std::exception {
};

class HttpSyntaxError : public HttpBaseError {
};

class HttpInvalidHeaders : public HttpSyntaxError {
};

class HttpInvalidQueryString : public HttpSyntaxError {
};

class HttpInvalidRequestLine : public HttpSyntaxError {
};

class HttpStandardError : public HttpBaseError {
  public:
    HttpStandardError(int code, std::string_view text);

    [[nodiscard]] const std::string &get_text() const;

    [[nodiscard]] int get_code() const;

    ~HttpStandardError() override = default;

  private:
    int code_{};
    std::string text_;
};

class HttpNotImplemented : public HttpStandardError {
  public:
    HttpNotImplemented();
    explicit HttpNotImplemented(std::string_view text);

};

}

#endif //TINYHTTP_TINYHTTP_ERRORS_H
