#ifndef UTIL_H
#define UTIL_H

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <string_view>

[[noreturn]] inline void die(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::fputs("fatal: ", stderr);
    std::vfprintf(stderr, fmt, args);
    std::fputc('\n', stderr);
    va_end(args);
    std::exit(EXIT_FAILURE);
}

bool parse_u64(const std::string_view s, uint64_t *out);

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#endif
