#include <cstdint>
#include <cstdio>

#include <SDL.h>

#include "verilated.h"

#include "display.h"
#include "panel.h"
#include "sim.h"

/* Cycles simulated between window title updates */
constexpr uint32_t SIM_STEP = 100000;
/* Window events are checked this many times per SIM_STEP */
constexpr uint32_t SIM_BATCHES = 10;
/* Window size relative to the panel */
constexpr int DISP_FACTOR = 2;

/* Uncomment to stop the simulation after this many cycles */
/* #define MAX_CYCLES 500000 */

int main(int argc, char *argv[]) {
    Verilated::commandArgs(argc, argv);

    Display display(DISP_WIDTH, DISP_HEIGHT, DISP_FACTOR);
    Sim sim;

    printf("Simulation start.\n");
    sim.reset(display);

    uint32_t ms_tick = SDL_GetTicks();
    bool running = true;
    while (running) {
        for (uint32_t batch = 0; batch < SIM_BATCHES; batch++) {
            for (uint32_t i = 0; i < SIM_STEP / SIM_BATCHES; i++)
                sim.tick(display);

            if (display.poll_quit())
                running = false;
        }

        uint32_t ms_delta = SDL_GetTicks() - ms_tick;
        char title[50];
        uint32_t khz = SIM_STEP / ms_delta;
        snprintf(title, sizeof(title), "Caster Sim (%u kHz)", khz);
        display.set_title(title);
        ms_tick = SDL_GetTicks();

#ifdef MAX_CYCLES
        if (sim.cycles() > MAX_CYCLES)
            break;
#endif
    }

    printf("Stop.\n");

    return 0;
}
