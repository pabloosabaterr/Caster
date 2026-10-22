#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "panel.h"
#include "display.h"

#include <SDL.h>

static uint8_t localbuf[DISP_WIDTH * DISP_HEIGHT];
static int8_t speedbuf[DISP_WIDTH * DISP_HEIGHT];

static int x_counter;
static int y_counter;
static int last_hs;

// #define USE_DBG_OUTPUT

#define ACCEL 20

void dispsim_set_pixel(uint32_t *pixels, int x, int y, uint8_t input) {
    if (x >= DISP_WIDTH)
        return;
    if (y >= DISP_HEIGHT)
        return;
#ifdef USE_DBG_OUTPUT
    int32_t pixel = input | (input << 4);
#else
    // Note: this is very wrong
    int32_t pixel = localbuf[y * DISP_WIDTH + x];
    int32_t speed = speedbuf[y * DISP_WIDTH + x];
    int8_t accel = (input == 1) ? (0 - ACCEL) : (input == 2) ? (ACCEL) : 0;

    speed += accel;
    pixel += speed;

    if (speed > 127)
        speed = 127;
    else if (speed < -128)
        speed = -128;
    // without external acceleration, it de-accelerate itself
    speed *= 0.9f;

    if (pixel > 255)
        pixel = 255;
    else if (pixel < 0)
        pixel = 0;

    // otherwise, nop
    localbuf[y * DISP_WIDTH + x] = pixel;
    speedbuf[y * DISP_WIDTH + x] = speed;
#endif

#ifdef K3
    int c = (x_counter + (DISP_HEIGHT - y_counter)) % 3;
    if (c == 0)
        pixel = 0xff000000 | (pixel << 16);
    else if (c == 1)
        pixel = 0xff000000 | pixel;
    else
        pixel = 0xff000000 | (pixel << 8);
#endif
#ifdef MONO
    pixel = (pixel << 16) | (pixel << 8) | (pixel) | 0xff000000ul;
#endif
    pixels[y * DISP_WIDTH + x] = pixel;
}

void dispsim_reset() {
    x_counter = 0;
    y_counter = 0;
    last_hs = 0;
    memset(localbuf, 0xff, sizeof(localbuf));
}

void dispsim_render(Display &display) {
    static float time_delta = 0.0f;
    static int last_ticks = SDL_GetTicks();
    const float TARGET_FPS = 60.0f;

    int cur_ticks = SDL_GetTicks();
    time_delta -=
        cur_ticks - last_ticks; // Actual ticks passed since last iteration
    time_delta += 1000.0f / TARGET_FPS; // Time allocated for this iteration
    last_ticks = cur_ticks;

    display.present();

    int time_to_wait = time_delta - (SDL_GetTicks() - last_ticks);
    if (time_to_wait > 0)
        SDL_Delay(time_to_wait);
}

void dispsim_apply(Display &display,
                   [[maybe_unused]] const uint8_t gdoe,
                   [[maybe_unused]] const uint8_t gdclk,
                   const uint8_t gdsp,
                   const uint8_t sdle,
                   [[maybe_unused]] const uint8_t sdoe,
                   const uint8_t sd,
                   const uint8_t sdce0,
                   [[maybe_unused]] const uint16_t dbg) {
    uint32_t *pixels = display.pixels();
    // SDLE = Hsync
    // GDSP = ~ Vsync
    // SDCE0 = ~ DE
    uint8_t hs = sdle;
    uint8_t vs = !gdsp;
    uint8_t de = !sdce0;
    static bool line_valid;

    if (!last_hs && hs) {
        x_counter = 0;
        if (vs) {
            y_counter = 0;
            line_valid = false;
            dispsim_render(display);
        } else {
            if (line_valid)
                y_counter++;
        }
    }

    if (de) {
#ifdef USE_DBG_OUTPUT
        dispsim_set_pixel(pixels, x_counter++, y_counter, (dbg >> 12) & 0xf);
        dispsim_set_pixel(pixels, x_counter++, y_counter, (dbg >> 8) & 0xf);
        dispsim_set_pixel(pixels, x_counter++, y_counter, (dbg >> 4) & 0xf);
        dispsim_set_pixel(pixels, x_counter++, y_counter, (dbg >> 0) & 0xf);
#else
        dispsim_set_pixel(pixels, x_counter++, y_counter, (sd >> 6) & 0x3);
        dispsim_set_pixel(pixels, x_counter++, y_counter, (sd >> 4) & 0x3);
        dispsim_set_pixel(pixels, x_counter++, y_counter, (sd >> 2) & 0x3);
        dispsim_set_pixel(pixels, x_counter++, y_counter, (sd >> 0) & 0x3);
#endif
        line_valid = true;
    }

    last_hs = hs;
}
