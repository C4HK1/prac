#include "mafia/core.h"

Action Action::promise_type::get_return_object() {
    return Action{ std::coroutine_handle<promise_type>::from_promise(*this) };
}

std::suspend_always Action::promise_type::initial_suspend() {
    return {};
}

std::suspend_always Action::promise_type::final_suspend() noexcept {
    return {};
}

void Action::promise_type::return_void() {
    // No return value for Action coroutines
}

void Action::promise_type::unhandled_exception() {
    // Silently ignore exceptions in Action coroutines
}

Action::~Action() {
    if (coroutine_handle) {
        coroutine_handle.destroy();
    }
}
