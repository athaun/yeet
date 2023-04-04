#pragma once

#include <fmt/core.h>
#include <fmt/format.h>
#include <string_view>

template<typename... Args>
inline auto print(fmt::format_string<Args...> fmt, Args&&... args) -> void {
    fmt::print(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto println(fmt::format_string<Args...> fmt, Args&&... args) -> void {
    fmt::print(fmt, std::forward<Args>(args)...);
    fmt::print("\n");
}

template<typename... Args>
inline auto eprint(fmt::format_string<Args...> fmt, Args&&... args) -> void {
    fmt::print(stderr, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto eprintln(fmt::format_string<Args...> fmt, Args&&... args) -> void {
    fmt::print(stderr, fmt, std::forward<Args>(args)...);
    fmt::print("\n");
}
