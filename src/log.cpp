#include "log.h"

#include <cstdarg>
#include <cstdio>

static LogLevel current = LogLevel::NORMAL;

void log_set_level(LogLevel level) {
    current = level;
}

void log_info(const char *fmt, ...) {
    if (current < LogLevel::NORMAL)
        return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

void log_debug(const char *fmt, ...) {
    if (current < LogLevel::VERBOSE)
        return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}
