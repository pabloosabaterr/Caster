#ifndef LOG_H
#define LOG_H

enum class LogLevel { QUIET = 0, NORMAL = 1, VERBOSE = 2 };

void log_set_level(LogLevel level);
void log_info(const char *fmt, ...);
void log_debug(const char *fmt, ...); /* --verbose */

#endif
