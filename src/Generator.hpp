//
// Created by koen on 7/22/26.
//

#ifndef RS_GENERATOR_H
#define RS_GENERATOR_H

#include <coroutine>
#include <optional>
#include <iterator>

template<typename T>
class Generator {
public:
    struct promise_type {
        std::optional<T> current_value;

        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }

        std::suspend_always yield_value(T value) {
            current_value = std::move(value);
            return {};
        }
        void return_void() {}
    };

    using handle_type = std::coroutine_handle<promise_type>;

    explicit Generator(handle_type h) : coro(h) {}
    ~Generator() { if (coro) coro.destroy(); }

    Generator(Generator&& other) noexcept : coro(other.coro) { other.coro = nullptr; }
    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (coro) coro.destroy();
            coro = other.coro;
            other.coro = nullptr;
        }
        return *this;
    }
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    // Range-based for support
    struct iterator {
        handle_type coro;
        bool operator!=(std::default_sentinel_t) const { return !coro.done(); }
        void operator++() { coro.resume(); }
        T operator*() const { return *coro.promise().current_value; }
    };

    iterator begin() {
        coro.resume();
        return iterator{coro};
    }
    std::default_sentinel_t end() { return {}; }

private:
    handle_type coro;
};

#endif //RS_GENERATOR_H
