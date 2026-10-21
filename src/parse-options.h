#ifndef PARSE_OPTIONS_H
#define PARSE_OPTIONS_H

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

enum class Option_type : uint8_t {
    /*
     * Sets the value variable to a boolean value.
     */
    BOOL = 0,
    /*
     * Stores argument as a unsigned 64-bit number.
     */
    U64,
};

struct Option {
    /*
     * Abbreviated form for the command example: "-n" for "--dry-run".
     */
    char abbrev;
    /*
     * Full command name.
     */
    std::string_view name;
    /*
     * Type of the command to know what it is expected of it.
     */
    Option_type type;
    /*
     * Pointer to a variable that will contain the value of that given opt after
     * parsing the args.
     *
     * Note: Once the number of options grows we should use std::variant
     */
    void *val;
};

#define OPT_BOOL(abbrev, name, var) {abbrev, name, Option_type::BOOL, var}
#define OPT_U64(abbrev, name, var)  {abbrev, name, Option_type::U64, var}
/*
 * Goes through the CLArgs, searching for matches with the options array.
 * Arguments starting with '+' are Verilator plusargs and are skipped.
 * Stops at the first argument that is not an option and returns its index.
 */
int parse_options(int argc, char **argv, const Option *options, size_t n);

#endif
