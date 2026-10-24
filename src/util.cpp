#include "util.h"
#include <charconv>
#include <string_view>
#include <system_error>

bool parse_u64(const std::string_view s, uint64_t *out) {
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), *out);
    return ec == std::errc() && ptr == s.data() + s.size();
}

