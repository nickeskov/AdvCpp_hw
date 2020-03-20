#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <cstddef>

class Process {
  public:

    explicit Process(const std::string &path);

    size_t write(const void *data, size_t len);

    void writeExact(const void *data, size_t len);

    size_t read(void *data, size_t len);

    void readExact(void *data, size_t len);

    bool isReadable() const;

    void closeStdin();

    ~Process() noexcept;

  private:
};

#endif //PROCESS_H
