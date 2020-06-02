#ifndef COROUTINE_COROUTINE_COROUTINE_H
#define COROUTINE_COROUTINE_COROUTINE_H

#include <cinttypes>
#include <functional>
#include <memory>

namespace Coroutine {

using routine_t = size_t;
using stack_t = std::unique_ptr<uint8_t[]>;
using routine_function_t = std::function<void()>;

routine_t create(routine_t id, const routine_function_t &function);

bool resume(routine_t id);

void kill(routine_t id, const std::exception_ptr &ptr);

void yield();

routine_t current();

void set_freelist_max_size(ssize_t new_size);

template<typename F, typename ...Args, typename = std::enable_if_t<!std::is_invocable_v<F>>>
routine_t create(F &&f, Args &&... args) {
    return create(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
}

template<typename F, typename ...Args>
bool create_and_run(F &&f, Args &&... args) {
    return resume(create(std::forward<F>(f), std::forward<Args>(args)...));
}


}

#endif //COROUTINE_COROUTINE_COROUTINE_H
