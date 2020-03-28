#include "utils.h"
#include "process.h"
#include "trivilog.h"

#include <array>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cerrno>

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

    proc = linuxproc::Process("/bin/bash", "/bin/bash");

    std::string_view cpuinfo = "/bin/cat /proc/cpuinfo\n";
    proc.write_exact(cpuinfo.data(), cpuinfo.length());
    proc.close_stdin();
    proc.close_stdin();

    ssize_t br = 0;
    while ((br = proc.read(buf.data(), buf.size() - 1)) == ssize_t(buf.size() - 1)) {
        buf[br] = '\0';
        std::cout << buf.data();
    }
    if (br == -1) {
        throw std::runtime_error(strerror(errno));
    }

    buf[br] = '\0';
    std::cout << buf.data();
}

void hw2_test() {
    auto log = trivilog::StdoutLogger();

    log.info("KEK");
    log.flush();
    log.set_level(trivilog::log_level::ERROR);

    log.info("THIS TEXT CANNOT BE DISPLAYED");

    auto filelog = trivilog::FileLogger("test.log");
    filelog.error("KEK");

    trivilog::global::Logger::warn("THIS IS DISPLAYED BY GLOBAL LOGGER");

    trivilog::global::Logger::get_instance().set_global_logger<trivilog::FileLogger>("test2.log");

    trivilog::global::Logger::warn("THIS IS DISPLAYED BY GLOBAL LOGGER");
}
