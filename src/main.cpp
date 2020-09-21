#include "utils.h"

#include <exception>
#include <iostream>

//#include "tinyhttp/server.h"
//#include "trivilog/safe_stdout_logger.h"

int main() {
    try {
        hw1_test();
        hw2_test();
        hw3_test();
        hw4_test();
        hw5_test();
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

//    std::cout << "---------------------------" << std::endl;
//
//    trivilog::StdoutLogger logger_;
//
//    tinyhttp::Server("127.0.0.1", 8000, logger_).run(1);

    return 0;
}
