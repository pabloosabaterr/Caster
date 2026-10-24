#include <cstdint>
#include <cstdio>

#include <SDL.h>
#include <optional>
#include <string_view>
#include <utility>

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

/* Window size in points, the panel is scaled to fit it */
constexpr int WIN_WIDTH = 640;
constexpr int WIN_HEIGHT = 480;
/* Window title update period, long enough to see several panel frames */
constexpr uint32_t TITLE_MS = 500;

constexpr int CASTER_SIM_VERSION[3] = {0, 1, 0};
/* clang-format off */
constexpr char HELP_MENU[] =
    "Caster simulator usage:\n"
    " -h, --help              show this help and exit\n"
    " -V, --version           show the version and exit\n"
    "     --headless          run without a window, needs --cycles\n"
    " -t, --trace             write a VCD trace to trace.vcd\n"
    "     --osd               show the OSD test pattern (default: off)\n"
    " -q, --quiet             print only errors\n"
    " -v, --verbose           print more details\n"
    " -c, --cycles <n>        stop after <n> clock cycles (default: never)\n"
    " -s, --source <source>   input to show (default: box):\n"
    "                           - box           20x20 texture bouncing around\n"
    "                           - fixed-border  black border on white\n"
    "                           - fixed-color   four gray bands\n"
    "                           - fixed-swap    gray bands swapped every 100 frames\n"
    "                           - blink         full black and white, alternating\n"
    "                           - image         swaps between test0.jpg and\n"
    "                                           test1.jpg scaled\n"
    " -p, --panel <panel>     panel type (default: mono):\n"
    "                           - mono\n"
    "                           - kaleido3\n"
    "     --panel-debug       draw Caster's dbg_wvfm_tgt signal instead of the\n"
    "                         simulated ink, mostly dark (default: off)\n"
/* TODO: add new option that can handle multiple args: --watch x,y */
    "     --watch-x <x>       with --watch-y, print the VRAM state of the pixel\n"
    "     --watch-y <y>       at x,y every time Caster reads or writes it\n"
    "     --digest            print a hash of the panel and VRAM output per frame\n"
    " -r, --resolution <w>x<h>\n"
    "                         panel resolution, the width a multiple of 4\n"
    "                         (default: 320x240)\n";
/* clang-format on */

struct SimOptions {
    bool help = false;
    bool version = false;
    bool headless = false;
    bool osd = false;
    bool trace = false;
    bool verbose = false;
    bool quiet = false;
    bool panel_dbg = false;
    bool digest = false;
    uint64_t cycles = 0;
    uint64_t watch_x = UINT64_MAX;
    uint64_t watch_y = UINT64_MAX;
    std::string_view source = "box";
    std::string_view panel = "mono";
    std::string_view resolution = "320x240";
};

static std::pair<uint64_t, uint64_t>
parse_resolution(std::string_view resolution) {
    size_t x = resolution.find('x');
    uint64_t w, h;
    if (x == std::string_view::npos ||
        !parse_u64(resolution.substr(0, x), &w) ||
        !parse_u64(resolution.substr(x + 1), &h))
        die("invalid size '%.*s', expected <width>x<height>",
            (int)resolution.size(),
            resolution.data());
    if (w == 0 || h == 0 || w > 4096 || h > 4096)
        die("size must be between 1 and 4096, got %llux%llu",
            (unsigned long long)w,
            (unsigned long long)h);
    if (w % 4)
        die("width must be a multiple of 4, got %llu", (unsigned long long)w);

    return {w, h};
};

static SimOptions opt;

int main(int argc, char *argv[]) {
    /* clang-format off */
    const Option options[] = {
        {'h', "help",        &opt.help},
        {0,   "headless",    &opt.headless},
        {'V', "version",     &opt.version},
        {0,   "osd",         &opt.osd},
        {'t', "trace",       &opt.trace},
        {'v', "verbose",     &opt.verbose},
        {'q', "quiet",       &opt.quiet},
        {'c', "cycles",      &opt.cycles},
        {'s', "source",      &opt.source},
        {'p', "panel",       &opt.panel},
        {0,   "panel-debug", &opt.panel_dbg},
        {0,   "watch-x",     &opt.watch_x},
        {0,   "watch-y",     &opt.watch_y},
        {0,   "digest",      &opt.digest},
        {'r', "resolution",  &opt.resolution},
    };
    /* clang-format on */

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

    PanelType panel_type = parse_choice("panel", opt.panel, PANEL_TYPES);
    SourceType source_type = parse_choice("source", opt.source, SOURCE_TYPES);
    std::pair<uint64_t, uint64_t> res = parse_resolution(opt.resolution);

    std::optional<std::pair<uint64_t, uint64_t>> watch;
    if ((opt.watch_x == UINT64_MAX) != (opt.watch_y == UINT64_MAX))
        die("--watch-x and --watch-y must be used together");

    if (opt.watch_x != UINT64_MAX) {
        if (opt.watch_x >= res.first || opt.watch_y >= res.second)
            die("--watch-x and --watch-y must be inside the %llux%llu panel",
                (unsigned long long)res.first,
                (unsigned long long)res.second);
        watch = {opt.watch_x, opt.watch_y};
    }

    Verilated::commandArgs(argc, argv);

    Display display(res.first, res.second, WIN_WIDTH, WIN_HEIGHT, opt.headless);
    Sim sim({
        .enable_trace = opt.trace,
        .show_osd = opt.osd,
        .source_type = source_type,
        .panel_type = panel_type,
        .watch = watch,
        .panel_debug = opt.panel_dbg,
        .digest = opt.digest,
        .res = res,
    });

    log_info("Simulation start.\n");
    sim.reset(display);

    uint32_t ms_tick = SDL_GetTicks();
    uint64_t last_frames = 0;
    uint64_t last_cycles = 0;
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

        uint32_t now = SDL_GetTicks();
        uint32_t ms_delta = now - ms_tick;
        if (ms_delta >= TITLE_MS) {
            uint64_t frames = sim.panel_frames();
            uint64_t cycles = sim.cycles();
            uint64_t khz = (cycles - last_cycles) / ms_delta;
            double fps = (frames - last_frames) * 1000.0 / ms_delta;

            char title[64];
            snprintf(title,
                     sizeof(title),
                     "Caster Sim (%llu kHz, %.1f fps)",
                     (unsigned long long)khz,
                     fps);
            display.set_title(title);

            last_frames = frames;
            last_cycles = cycles;
            ms_tick = now;
        }
    }

    sim.finish_digest();
    log_info("Stop.\n");

    return 0;
}
