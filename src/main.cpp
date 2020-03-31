#include "utils.h"

#include <exception>

int main() {
    try {
        hw1_test();
    }
    catch (std::exception &) {
        return 1;
    }
    return 0;
}
