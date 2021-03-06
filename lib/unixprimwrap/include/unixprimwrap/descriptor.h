#ifndef UNIXPRIMWRAP_UNIXPRIMWRAP_DESCRIPTOR_H
#define UNIXPRIMWRAP_UNIXPRIMWRAP_DESCRIPTOR_H

namespace unixprimwrap {

class Descriptor {
  public:
    Descriptor() noexcept = default;

    explicit Descriptor(int &fd) noexcept;

    explicit Descriptor(int &&fd) noexcept;

    Descriptor(const Descriptor &other);

    Descriptor &operator=(const Descriptor &other);

    Descriptor(Descriptor &&other) noexcept;

    Descriptor &operator=(Descriptor &&other) noexcept;

    void swap(Descriptor &other) noexcept;

    [[nodiscard]] int data() const noexcept;

    explicit operator int() const noexcept;

    [[nodiscard]] bool is_valid() const noexcept;

    [[nodiscard]] int extract() noexcept;

    int close() noexcept;

    [[nodiscard]] int dup2(int fd) const noexcept;

    ~Descriptor() noexcept;

  private:
    int fd_ = -1;
};

}

#endif //UNIXPRIMWRAP_UNIXPRIMWRAP_DESCRIPTOR_H
