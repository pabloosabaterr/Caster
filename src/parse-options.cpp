#include "parse-options.h"
#include "util.h"

#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>

static bool parse_u64(const char *s, uint64_t *out) {
    if (!isdigit(static_cast<unsigned char>(*s)))
        return false;

    char *end;
    errno = 0;
    unsigned long long v = strtoull(s, &end, 10);
    if (errno || *end != '\0')
        return false;

    *out = v;
    return true;
}

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
                value = argv[i] + 2 + eq + 1;
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

        std::visit(
            overloaded{
                [&](bool *p) {
                    if (value)
                        die("option '%s' takes no value", argv[i]);
                    *p = true;
                },
                [&](uint64_t *p) {
                    if (!parse_u64(value, p))
                        die("invalid number '%s' for '%s'", value, argv[i]);
                },
            },
            found->val);
    }

    return i;
}
