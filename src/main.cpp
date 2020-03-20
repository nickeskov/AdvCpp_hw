#include "process.h"

int main() {
    char buf[10] = {'\0'};

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

    return 0;
}
