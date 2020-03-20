#include "process.h"

int main() {
    char buf[100] = {'\0'};

    auto proc = linuxproc::Process("/bin/cat", std::string_view("/bin/cat"));

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

    char cpuinfo[] = "/bin/cat /proc/cpuinfo";
    proc.write_exact(cpuinfo, sizeof(cpuinfo) - 1);
    proc.close_stdin();

    int br = -1;
    while ((br = proc.read(buf, sizeof(buf) - 1)) > 0) {
        buf[br] = '\0';
        std::cout << buf;
    }

    return 0;
}
