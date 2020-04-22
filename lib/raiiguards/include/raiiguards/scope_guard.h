#ifndef RAIIGUARDS_RAIIGUARDS_SCOPE_GUARD_H
#define RAIIGUARDS_RAIIGUARDS_SCOPE_GUARD_H

#include <type_traits>
#include <functional>

namespace raiiguards {

using scope_guard_handler_t = std::function<void()>;

class ScopeGuard {
  public:
    explicit ScopeGuard(scope_guard_handler_t handler);

    ScopeGuard(const ScopeGuard &) = delete;

    ScopeGuard &operator=(const ScopeGuard &) = delete;

    ScopeGuard(ScopeGuard &&) = delete;

    ScopeGuard &operator=(ScopeGuard &&) = delete;

    ~ScopeGuard() noexcept;

  private:
    scope_guard_handler_t scope_guard_handler_;
};

}

#endif //RAIIGUARDS_RAIIGUARDS_SEM_LOCK_GUARD_H
