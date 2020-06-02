#include "coroutine/coroutine.h"

#include <deque>
#include <memory>
#include <map>
#include <utility>
#include <stdexcept>

extern "C" {
#include <ucontext.h>
}

namespace Coroutine {

class Routine;

namespace {

constexpr inline size_t ROUTINE_STACK_SIZE = 1u << 16u;

thread_local struct Ordinator {
    std::map<routine_t, Routine> routines;
    std::deque<decltype(routines)::node_type> finished;
    routine_t current = 0;
    ucontext_t ctx{};
    ssize_t freelist_max_size = -1;
} ordinator; // NOLINT (nickeskov)
// Initialization with thread_local storage
// duration may throw an exception that cannot be caught

void entry();

}

class Routine {
  public:
    explicit Routine(routine_function_t f)
            : func(std::move(f)), stack(std::make_unique<uint8_t[]>(ROUTINE_STACK_SIZE)) {
        getcontext(&ctx);
        ctx.uc_stack.ss_sp = stack.get();
        ctx.uc_stack.ss_size = ROUTINE_STACK_SIZE;
        ctx.uc_link = &ordinator.ctx;
        makecontext(&ctx, entry, 0);
    }

    Routine(const Routine &) = delete;

    Routine &operator=(const Routine &) = delete;

    void reset(const routine_function_t &f) {
        func = f;
        finished = false;
        exception = {};
        makecontext(&ctx, entry, 0);
    }

  public:
    routine_function_t func;
    stack_t stack;
    bool finished = false;
    ucontext_t ctx{};
    std::exception_ptr exception{};
};

routine_t create(routine_t id, const routine_function_t &function) {
    auto &o = ordinator;
    if (o.finished.empty()) {
        o.routines.emplace(std::make_pair(id, function));
        return id;
    } else {
        auto node = std::move(o.finished.back());
        o.finished.pop_back();

        node.key() = id;
        node.mapped().reset(function);

        o.routines.insert(std::move(node));
        return id;
    }
}

bool resume(routine_t id) {
    if (id == 0) {
        return false;
    }

    auto &o = ordinator;

    const auto &routine_ref = o.routines.at(id);
    if (routine_ref.finished) {
        throw std::logic_error("finished routine in map of active routines");
    }

    o.current = id;
    if (swapcontext(&o.ctx, &routine_ref.ctx) < 0) {
        o.current = 0;
        return false;
    }

    if (routine_ref.finished) {
        auto exception_ptr = routine_ref.exception;

        auto routine = o.routines.extract(id);
        if (o.freelist_max_size < 0
            || o.finished.size() < static_cast<size_t>(o.freelist_max_size)) {
            o.finished.emplace_back(std::move(routine));
        }

        if (exception_ptr) {
            std::rethrow_exception(exception_ptr);
        }
    }

    return true;
}

void kill(routine_t id, const std::exception_ptr &ptr) {
    if (id == 0) {
        throw std::invalid_argument("try killing main routine");
    }
    if (!ptr) {
        throw std::invalid_argument("emtpy exception ptr");
    }

    auto &o = ordinator;

    auto &routine_ref = o.routines.at(id);
    if (routine_ref.finished) {
        throw std::logic_error("finished routine in map of active routines");
    }

    routine_ref.exception = ptr;

    resume(id); // must throw exception

    throw std::logic_error("resume not throw exception");
}

void yield() {
    auto &o = ordinator;

    routine_t id = o.current;
    if (id == 0) {
        throw std::logic_error("trying call yield in main routine");
    }

    auto &routine_ref = o.routines.at(id);

    o.current = 0;
    swapcontext(&routine_ref.ctx, &o.ctx);

    if (routine_ref.exception) {
        std::rethrow_exception(routine_ref.exception);
    }
}

routine_t current() {
    return ordinator.current;
}

void set_freelist_max_size(ssize_t new_size) {
    auto &o = ordinator;

    o.freelist_max_size = new_size;

    if (new_size >= 0) {
        o.finished.resize(new_size);
        o.finished.shrink_to_fit();
    }
}

namespace {

void entry() {
    auto &o = ordinator;
    routine_t id = o.current;
    auto &routine_ref = o.routines.at(id);

    if (routine_ref.func) {
        try {
            routine_ref.func();
        } catch (...) {
            routine_ref.exception = std::current_exception();
        }
    }

    routine_ref.finished = true;
    o.current = 0;
}

}

}
