#include "process.h"

//#include <utility>
//#include <tuple>
//#include <array>
//#include <algorithm>
//#include <initializer_list>
//
//class Test {
//  public:
//
//    template<typename ...Args>
//    using pack_t = typename std::common_type<typename std::decay<Args>::type ...>::type;
//
//    template<typename ...Args,
//            typename = std::enable_if_t<
//                    std::is_convertible_v<
//                            pack_t<Args...>,
//                            std::string_view
//                    >, std::string_view>>
//    explicit Test(std::string_view path, Args &&... args) {
//        std::initializer_list<std::string_view> args_views = {
//                std::forward<Args>(args)...,
//                nullptr
//        };
//        // NOLINTNEXTLINE: Redundant initialization
//        std::array<const char *, sizeof...(Args) + 1> args_cstrs;
//        std::transform(args_views.begin(), args_views.end(), args_cstrs.begin(), [](const auto &str_view) {
//            return str_view.data();
//        });
//
//        auto proc = linuxproc::Process(path, const_cast<char ** const>(args_cstrs.data()));
//    }
//};

int main() {
    char buf[100] = "";

    auto proc = linuxproc::Process("/bin/cat", "/bin/cat");

    proc.write("lollol", 6);
    proc.read(buf, 6);
    std::cout << buf << std::endl;

    proc.write_exact("12345678", 8);
    proc.close_stdin();

    proc.read_exact(buf, 8);
    std::cout << buf << std::endl;


    std::cout << std::boolalpha;
    std::cout << proc.is_readable() << std::endl;

    proc = linuxproc::Process("/bin/bash", std::string_view("/bin/bash"));

    std::string_view cpuinfo = "/bin/cat /proc/cpuinfo\n";
    proc.write_exact(cpuinfo.data(), cpuinfo.length());
    proc.close_stdin();
    proc.close_stdin();

    int br = 0;
    while ((br = proc.read(buf, sizeof(buf) - 1)) == sizeof(buf) - 1) {
        buf[br] = '\0';
        std::cout << buf;
    }
    buf[br] = '\0';
    std::cout << buf;

    return 0;
}
