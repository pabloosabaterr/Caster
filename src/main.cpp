/*
 * Caster simulator
 * Copyright 2021 Wenting Zhang
 * Copyright 2026 Pablo Sabater
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
