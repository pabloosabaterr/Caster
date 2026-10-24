#include "parse-options.h"
#include "util.h"

#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <string.h>
#include <string_view>
#include <variant>

static constexpr size_t LONG_PREFIX_SIZE = 2;
static constexpr size_t SHORT_PREFIX_SIZE = 1;

int parse_options(int argc, char **argv, const Option *options, size_t n) {
    int i;

    for (i = 1; i < argc; i++) {
        const Option *found = nullptr;
        const char *value = nullptr;

        /* Verilator plusargs */
        if (argv[i][0] == '+')
            continue;

        /*
         * Only accept abbrev and full options: "-f", "--foo".
         */
        if (*argv[i] != '-' || argv[i][1] == '\0' ||
            (argv[i][1] == '-' && argv[i][2] == '\0'))
            break;

        if (argv[i][1] == '-') { /* Full name */
            std::string_view name = argv[i] + 2;

            size_t eq = name.find('=');
            if (eq != std::string_view::npos) {
                value = argv[i] + LONG_PREFIX_SIZE + eq + SHORT_PREFIX_SIZE;
                name = name.substr(0, eq);
            }

            for (size_t j = 0; j < n; j++) {
                if (!options[j].name.empty() && options[j].name == name) {
                    found = &options[j];
                    break;
                }
            }
        } else if (argv[i][2] == '\0') { /* Abbrev */
            char abbrev = argv[i][1];

            for (size_t j = 0; j < n; j++) {
                if (options[j].abbrev && options[j].abbrev == abbrev) {
                    found = &options[j];
                    break;
                }
            }
        }

        if (!found)
            die("unknown option '%s'", argv[i]);

        const char *opt_name = argv[i];
        bool needs_val = !std::holds_alternative<bool *>(found->val);
        if (needs_val && !value) {
            if (i + 1 >= argc)
                die("option '%s' needs a value", opt_name);
            value = argv[++i];
        }

        std::visit(
            overloaded{
                [&](bool *p) {
                    if (value)
                        die("option '%s' takes no value", opt_name);
                    *p = true;
                },
                [&](uint64_t *p) {
                    if (!parse_u64(value, p))
                        die("invalid number '%s' for '%s'", value, opt_name);
                },
                [&](std::string_view *p) {
                    if (*value == '\0')
                        die("option '%s' needs a value", opt_name);
                    *p = value;
                },
            },
            found->val);
    }

    return i;
}
