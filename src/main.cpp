#include "utils.h"

#include <exception>
#include <iostream>

int main() {
    try {
        hw1_test();
        hw2_test();
        hw3_test();
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
