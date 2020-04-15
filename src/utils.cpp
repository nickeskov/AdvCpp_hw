#include "utils.h"
#include "process.h"
#include "trivilog.h"
#include "tcp_sync_connection.h"
#include "tcp_sync_server.h"

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

    trivilog::global::Logger::set_global_logger<trivilog::FileLogger>("test2.log");

    trivilog::global::Logger::warn("THIS IS DISPLAYED BY GLOBAL LOGGER");

    auto logptr = trivilog::create_logger<trivilog::StderrLogger>();

    trivilog::global::Logger::set_global_logger(std::move(logptr));

    trivilog::global::Logger::warn("THIS IS DISPLAYED BY NEW GLOBAL LOGGER");
}

void hw3_test() {
    char test_str[] = "1234567890";
    char buff[sizeof(test_str)]{};

    auto server = tcpcon::ipv4::sync::Server("127.0.0.1", 0);

    std::cout << "started server on "
              << server.get_src_addr() << ":" << server.get_src_port() << std::endl;

    auto write_client1 = tcpcon::ipv4::sync::Connection("127.0.0.1", server.get_src_port());
    write_client1.set_write_timeout(1);
    auto write_client2 = tcpcon::ipv4::sync::Connection("127.0.0.1", server.get_src_port());
    write_client2.set_write_timeout(1);

    write_client1.write_exact(test_str, sizeof(test_str));
    std::cout << "started write_client1=" << write_client1.to_string() << std::endl;

    write_client2.write_exact(test_str, sizeof(test_str));
    std::cout << "started write_client2=" << write_client2.to_string() << std::endl;

    auto read_client1 = server.accept();
    std::cout << "accepted read_client1=" << read_client1.to_string() << std::endl;
    read_client1.set_read_timeout(1);

    auto read_client2 = server.accept();
    std::cout << "accepted read_client2=" << read_client2.to_string() << std::endl;
    read_client2.set_read_timeout(1);

    read_client1.read_exact(buff, sizeof(test_str));
    if (strcmp(buff, test_str) != 0) {
        throw std::runtime_error("hw3 test failed");
    }

    read_client2.read_exact(buff, sizeof(test_str));
    if (strcmp(buff, test_str) != 0) {
        throw std::runtime_error("hw3 test failed");
    }

    write_client1.close();
    if (read_client1.read(buff, 1) != 0 || read_client1.is_readable()) {
        throw std::runtime_error("hw3 test failed");
    }

    write_client2.close();
    if (read_client2.read(buff, 1) != 0 || read_client2.is_readable()) {
        throw std::runtime_error("hw3 test failed");
    }

    read_client1.close();

    read_client2.close();

    server.close();
}
