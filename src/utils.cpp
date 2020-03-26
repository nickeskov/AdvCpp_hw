#include "utils.h"
#include "process.h"

#include <array>
#include <iostream>

void hw1_test() {
    std::array<char, 100> buf{};

    auto proc = linuxproc::Process("/bin/cat", "/bin/cat");

    proc.write("lollol", 6);
    proc.read(buf.data(), 6);
    std::cout << buf.data() << std::endl;

    proc.write_exact("12345678", 8);
    proc.close_stdin();

    proc.read_exact(buf.data(), 8);
    std::cout << buf.data() << std::endl;


    std::cout << std::boolalpha;
    std::cout << proc.is_readable() << std::endl;

    proc = linuxproc::Process("/bin/bash", std::string_view("/bin/bash"));

    std::string_view cpuinfo = "/bin/cat /proc/cpuinfo\n";
    proc.write_exact(cpuinfo.data(), cpuinfo.length());
    proc.close_stdin();
    proc.close_stdin();

    ssize_t br = 0;
    while ((br = proc.read(buf.data(), buf.size() - 1)) == buf.size() - 1) {
        buf[br] = '\0';
        std::cout << buf.data();
    }
    buf[br] = '\0';
    std::cout << buf.data();
}