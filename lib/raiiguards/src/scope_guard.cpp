#include "raiiguards/scope_guard.h"

#include <utility>

namespace raiiguards {

ScopeGuard::~ScopeGuard() noexcept {
    if (scope_guard_handler_) {
        try {
            scope_guard_handler_();
        } catch (...) {}
    }
}

ScopeGuard::ScopeGuard(scope_guard_handler_t handler)
        : scope_guard_handler_(std::move(handler)) {}

}
