#include "utils.h"
#include "unixprimwrap/fork.h"

#ifdef HW_ENABLE_HW1

#include "linuxproc/process.h"

#endif

#ifdef HW_ENABLE_HW2

#include "trivilog/trivilog.h"

#endif

#ifdef HW_ENABLE_HW3

#include "tcpcon/sync/connection.h"
#include "tcpcon/sync/server.h"

#endif

#ifdef HW_ENABLE_HW4

#include "tcpcon/async/connection.h"
#include "tcpcon/async/epoll/server.h"

extern "C" {
#include <sys/epoll.h>
}
#endif

#ifdef HW_ENABLE_HW5

#include "shmem/shared_memory.h"
#include "shmem/allocators/linear_allocator.h"
#include "shmem/containers/map.h"
#include "shmem/types.h"

#endif

#include <array>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <chrono>
#include <thread>

void hw1_test() {
#ifdef HW_ENABLE_HW1

    std::cout << "---------hw1 test---------" << std::endl;
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

#endif
}

void hw2_test() {
#ifdef HW_ENABLE_HW2

    std::cout << "---------hw2 test---------" << std::endl;

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

#endif
}

void hw3_test() {
#ifdef HW_ENABLE_HW3

    std::cout << "---------hw3 test---------" << std::endl;

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

#endif
}

void hw4_test() {
#ifdef HW_ENABLE_HW4

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

            cfg.max_accept_clients_per_loop = 3;
            cfg.epoll_accept_flags = EPOLLIN;
            cfg.epoll_server_flags = EPOLLIN;
            cfg.epoll_sigmask = nullptr;
            cfg.epoll_max_events = 16;
            cfg.epoll_timeout = -1;

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

    auto sleep_duration = std::chrono::milliseconds(100); // 100 millisecond

    while (client1.read_in_io_buff(max_msg_len) < 0) {
        std::this_thread::sleep_for(sleep_duration);
    }

    while (client2.read_in_io_buff(max_msg_len) < 0) {
        std::this_thread::sleep_for(sleep_duration);
    }

    if (client1.get_io_buffer() != test_str) {
        throw std::runtime_error("hw4 test failed");
    }

    if (client2.get_io_buffer() != test_str) {
        throw std::runtime_error("hw4 test failed");
    }

    client1.close();
    client2.close();

    std::this_thread::sleep_for(sleep_duration);

#endif
}

#ifdef HW_ENABLE_HW5

namespace {

namespace memsize {

enum : size_t {
    BYTE = sizeof(std::byte),
    KILOBYTE = BYTE * 1024,
    MEGABYTE = KILOBYTE * 1024,
    GIGABYTE = MEGABYTE * 1024,
    TERABYTE = GIGABYTE * 1024,
    PETABYTE = TERABYTE * 1024
};

}

}

#endif

void hw5_test() {
#ifdef HW_ENABLE_HW5

    std::cout << "---------hw5 test---------" << std::endl;

    using shstring = typename shmem::types::string<shmem::allocators::LinearAllocator<char>>;

    // ------------------ int->int

    auto shmem_ptr = shmem::create_shmem<std::byte>(memsize::KILOBYTE);
    shmem::allocators::LinearAllocator<std::byte>
            shallocator{shmem_ptr.get(), memsize::KILOBYTE};

    shmem::allocators::LinearAllocator<std::pair<const int, int>> pair_allocator{shallocator};

    shmem::containers::Map<int, int, std::less<>, decltype(pair_allocator)>
            shmap{pair_allocator, true};

    // ------------------ string->string

    auto shmem_ptr1 = shmem::create_shmem<std::byte>(memsize::KILOBYTE);
    shmem::allocators::LinearAllocator<std::byte>
            shallocator1{shmem_ptr1.get(), memsize::KILOBYTE};

    shmem::allocators::LinearAllocator<std::pair<const shstring, shstring>> pair_allocator1{shallocator};

    shmem::containers::Map<shstring, shstring, std::less<>, decltype(pair_allocator1)>
            shmap1{pair_allocator1, true};

    // ------------------ string->int

    auto shmem_ptr2 = shmem::create_shmem<std::byte>(memsize::KILOBYTE);
    shmem::allocators::LinearAllocator<std::byte>
            shallocator2{shmem_ptr2.get(), memsize::KILOBYTE};

    shmem::allocators::LinearAllocator<std::pair<const shstring, int>> pair_allocator2{shallocator};

    shmem::containers::Map<shstring, int, std::less<>, decltype(pair_allocator2)>
            shmap2{pair_allocator2, true};

    // ------------------

    unixprimwrap::Fork child_fork;
    if (!child_fork.is_valid()) {
        throw std::runtime_error("hw4: fork failed");
    }

    constexpr auto int_key = 100500;
    constexpr auto very_long_string_key = "This is a very long string key for my shared map testing!";

    if (child_fork.is_child()) {
        shmap.insert(int_key, int_key);
        shmap1.insert(very_long_string_key, "Yeah!!! It's working!!!");
        shmap2.insert(very_long_string_key, int_key);
        return;
    }

    auto sleep_duration = std::chrono::milliseconds(100); // 100 millisecond
    std::this_thread::sleep_for(sleep_duration);

    auto value1 = shmap.at(int_key);
    auto value2 = shmap1.at<std::string>(very_long_string_key);
    auto value3 = shmap2.at(very_long_string_key);

    shmap.destroy();
    shmap1.destroy();
    shmap2.destroy();

    std::cout << value1 << std::endl;
    std::cout << value2 << std::endl;
    std::cout << value3 << std::endl;

#endif
}
