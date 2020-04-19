#include "utils.h"
#include "linuxproc/process.h"
#include "trivilog/trivilog.h"
#include "tcpcon/sync/connection.h"
#include "tcpcon/sync/server.h"
#include "tcpcon/async/connection.h"
#include "tcpcon/async/epoll/server.h"
#include <unixprimwrap/fork.h>

#include <array>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <ctime>

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

    auto server = tcpcon::sync::ipv4::Server("127.0.0.1", 0);

    std::cout << "started server on "
              << server.get_src_addr() << ":" << server.get_src_port() << std::endl;

    auto write_client1 = tcpcon::sync::ipv4::Connection("127.0.0.1", server.get_src_port());
    std::cout << "started write_client1=" << write_client1.to_string() << std::endl;
    write_client1.set_write_timeout(1);

    auto write_client2 = tcpcon::sync::ipv4::Connection("127.0.0.1", server.get_src_port());
    std::cout << "started write_client2=" << write_client2.to_string() << std::endl;
    write_client2.set_write_timeout(1);

    write_client1.write_exact(test_str, sizeof(test_str));

    write_client2.write_exact(test_str, sizeof(test_str));

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

void hw4_test() {
    std::cout << "---------hw4 test---------" << std::endl;

    char test_str[] = "1234567890";
    const int max_msg_len = sizeof(test_str);

    auto server = tcpcon::async::ipv4::Server("127.0.0.1", 0);
    std::cout << "started server on "
              << server.get_src_addr() << ":" << server.get_src_port() << std::endl;

    // forks here
    unixprimwrap::Fork server_fork;

    if (!server_fork.is_valid()) {
        throw std::runtime_error("hw4: fork failed");
    }
    if (server_fork.is_child()) {
        const int max_len_per_call = 256;
        auto connection_handler = [&](tcpcon::async::ipv4::Connection &conn, uint32_t events) {
            try {
                if (events & EPOLLIN) {
                    if (conn.read_in_io_buff(max_len_per_call) == 0) {
                        server.close_connection(conn, events);
                    } else {
                        conn.write_from_io_buff(max_len_per_call);
                    }
                }
            } catch (std::exception &e) {
                std::cerr << "hw4: error in server process, error=" << e.what()
                          << ", errno=" << strerror(errno) << std::endl;
            }
        };
        auto after_accept_handler = [](tcpcon::async::ipv4::Connection &conn, uint32_t events) {
            std::cout << "accepted connection=" << conn.to_string()
                      << ", server accept events=" << events << std::endl;
        };
        auto before_close_handler = [](const tcpcon::async::ipv4::Connection &conn, uint32_t events) {
            std::cout << "closing connection=" << conn.to_string()
                      << ", closed by events=" << events << std::endl;
        };
        try {
            server.set_after_accept_handler(after_accept_handler);
            server.set_before_close_handler(before_close_handler);

            auto cfg = tcpcon::async::ipv4::Server::EventLoopConfig();
            server.event_loop(connection_handler, cfg);
        } catch (std::exception &e) {
            std::cerr << "hw4: error in server, error=" << e.what()
                      << ", errno=" << strerror(errno);
        }
        return;
    }
    uint16_t server_port = server.get_src_port();
    server.close(0);

    auto client1 = tcpcon::async::ipv4::Connection("127.0.0.1", server_port);
    std::cout << "started write_client1=" << client1.to_string() << std::endl;
    client1.set_write_timeout(1);
    client1.set_read_timeout(1);

    auto client2 = tcpcon::async::ipv4::Connection("127.0.0.1", server_port);
    std::cout << "started write_client2=" << client1.to_string() << std::endl;
    client2.set_write_timeout(1);
    client2.set_read_timeout(1);

    client1.get_io_buffer() += test_str;
    client2.get_io_buffer() += test_str;

    client1.write_from_io_buff(max_msg_len);
    client2.write_from_io_buff(max_msg_len);

    struct timespec timespec{};
    timespec.tv_nsec = 10000000; // 10 millisecond

    while (client1.read_in_io_buff(max_msg_len) < 0) {
        nanosleep(&timespec, nullptr);
    }

    while (client2.read_in_io_buff(max_msg_len) < 0) {
        nanosleep(&timespec, nullptr);
    }

    if (client1.get_io_buffer() != test_str) {
        throw std::runtime_error("hw4 test failed");
    }

    if (client2.get_io_buffer() != test_str) {
        throw std::runtime_error("hw4 test failed");
    }

    client1.close();
    client2.close();

    nanosleep(&timespec, nullptr);
}
