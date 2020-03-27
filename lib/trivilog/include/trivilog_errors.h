#ifndef HW_TRIVILOG_ERRORS_H
#define HW_TRIVILOG_ERRORS_H

#include <exception>

namespace trivilog {

class BaseError : public std::exception {};

class InvalidLogLevelError : public BaseError {};

}

#endif //HW_TRIVILOG_ERRORS_H
