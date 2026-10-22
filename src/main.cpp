#include <cstdint>
#include <cstdio>

#include <SDL.h>
#include <string_view>

#include "verilated.h"

#include "display.h"
#include "panel.h"
#include "parse-options.h"
#include "sim.h"
#include "util.h"
#include "log.h"
#include "video-source.h"

/* Cycles simulated between window title updates */
constexpr uint32_t SIM_STEP = 100000;
/* Window events are checked this many times per SIM_STEP */
constexpr uint32_t SIM_BATCHES = 10;
/* Window size relative to the panel */
constexpr int DISP_FACTOR = 2;

constexpr int CASTER_SIM_VERSION[3] = {0, 1, 0};
constexpr char HELP_MENU[] =
    "Caster simulator usage:\n"
    " -h, --help       show this help and exit\n"
    " -V, --version    show the version and exit\n"
    "     --headless   run without a window\n"
    " -t, --trace      write a VCD trace to trace.vcd\n"
    "     --osd        show the OSD test pattern\n"
    " -q, --quiet      print only errors\n"
    " -v, --verbose    print more details\n"
    " -c, --cycles N   stop after N clock cycles\n"
    " -s, --source SRC input to show, one of: bounce (default),\n"
    "                  border, bars, bars-flip, flash, images, stdin\n";

struct SimOptions {
    bool help = false;
    bool version = false;
    bool headless = false;
    bool osd = false;
    bool trace = false;
    bool verbose = false;
    bool quiet = false;
    uint64_t cycles = 0;
    std::string_view source = "box";
};

static SimOptions opt;

int main(int argc, char *argv[]) {
    const Option options[] = {
        {'h', "help",     &opt.help},
        {0,   "headless", &opt.headless},
        {'V', "version",  &opt.version},
        {0,   "osd",      &opt.osd},
        {'t', "trace",    &opt.trace},
        {'v', "verbose",  &opt.verbose},
        {'q', "quiet",    &opt.quiet},
        {'c', "cycles",   &opt.cycles},
        {'s', "source",   &opt.source},
    };

    int argi = parse_options(argc, argv, options, std::size(options));
    if (argi < argc)
        die("unexpected argument '%s'", argv[argi]);

    if (opt.help) {
        printf("%s", HELP_MENU);
        return 0;
    }

    if (opt.version) {
        printf("Caster simulator %d.%d.%d\n",
               CASTER_SIM_VERSION[0],
               CASTER_SIM_VERSION[1],
               CASTER_SIM_VERSION[2]);
        return 0;
    }

    if (opt.verbose && opt.quiet)
        die("--verbose and --quiet cannot be used together");

    if (opt.quiet)
        log_set_level(LogLevel::QUIET);

    if (opt.verbose)
        log_set_level(LogLevel::VERBOSE);

    if (opt.headless && !opt.cycles)
        die("--headless needs --cycles, it would never stop");

    srcsim_set_source(opt.source);

    Verilated::commandArgs(argc, argv);

    Display display(DISP_WIDTH, DISP_HEIGHT, DISP_FACTOR, opt.headless);
    Sim sim(opt.trace, opt.osd);

    log_info("Simulation start.\n");
    sim.reset(display);

    uint32_t ms_tick = SDL_GetTicks();
    bool running = true;
    while (running) {
        for (uint32_t batch = 0; batch < SIM_BATCHES && running; batch++) {
            for (uint32_t i = 0; i < SIM_STEP / SIM_BATCHES; i++) {
                if (opt.cycles && sim.cycles() >= opt.cycles) {
                    running = false;
                    break;
                }
                sim.tick(display);
            }

            if (display.poll_quit())
                running = false;
        }

        uint32_t ms_delta = SDL_GetTicks() - ms_tick;
        if (ms_delta) {
            char title[50];
            uint32_t khz = SIM_STEP / ms_delta;
            snprintf(title, sizeof(title), "Caster Sim (%u kHz)", khz);
            display.set_title(title);
        }
        ms_tick = SDL_GetTicks();
    }

    log_info("Stop.\n");

    return 0;
}
