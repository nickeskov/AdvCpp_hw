#ifndef TRIVILOG_TRIVILOG_ERRORS_H
#define TRIVILOG_TRIVILOG_ERRORS_H

#include <exception>

namespace trivilog {

class BaseError : public std::exception {};

class InvalidLogLevelError : public BaseError {};

}

#endif //TRIVILOG_TRIVILOG_ERRORS_H
