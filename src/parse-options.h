#ifndef PARSE_OPTIONS_H
#define PARSE_OPTIONS_H

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <variant>

struct Option {
    /* Abbreviated form for the command example: "-n" for "--dry-run". */
    char abbrev;
    /* Full command name. */
    std::string_view name;
    /*
     * Pointer to a variable that will contain the value of that given opt after
     * parsing the args.
     */
    std::variant<bool *, uint64_t *, std::string_view *> val;
};

/*
 * Goes through the CLArgs, searching for matches with the options array.
 * Arguments starting with '+' are Verilator plusargs and are skipped.
 * Stops at the first argument that is not an option and returns its index.
 */
int parse_options(int argc, char **argv, const Option *options, size_t n);

#endif
