#include <cstdint>
#include <stdio.h>
#include <string.h>

#include "panel.h"
#include "display.h"

#include <SDL.h>

#define ACCEL 20

Panel::Panel(PanelType type, bool dbg) : type(type), dbg(dbg) {
    reset();
}

void Panel::set_pixel(uint32_t *pixels, int x, int y, uint8_t input) {
    int32_t pixel;
    if (x >= DISP_WIDTH)
        return;
    if (y >= DISP_HEIGHT)
        return;

    if (dbg) {
        pixel = input | (input << 4);
    } else {
        /*
         * Very rough model of the ink: each pixel has a level and a speed, the
         * drive from Caster accelerates it towards black or white and friction
         * slows it down. ACCEL and the 0.9 friction are not based on any real
         * panel, so transition times and ghosting are only indicative.
         */
        pixel = localbuf[y * DISP_WIDTH + x];
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
    }

    switch (type) {
    case PanelType::KALEIDO3: {
        int c = (x + (DISP_HEIGHT - y)) % 3;
        if (c == 0)
            pixel = 0xff000000 | (pixel << 16);
        else if (c == 1)
            pixel = 0xff000000 | pixel;
        else
            pixel = 0xff000000 | (pixel << 8);
        break;
    }
    case PanelType::MONOCHROME: {
        pixel = (pixel << 16) | (pixel << 8) | (pixel) | 0xff000000ul;
        break;
    }
    }
    pixels[y * DISP_WIDTH + x] = pixel;
}

void Panel::reset() {
    x_counter = 0;
    y_counter = 0;
    last_hs = 0;
    memset(localbuf, 0xff, sizeof(localbuf));
    last_ticks = SDL_GetTicks();
    line_valid = false;
    time_delta = 0;
    memset(speedbuf, 0, sizeof(speedbuf));
}

void Panel::render(Display &display) {
    if (display.is_headless())
        return;

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

void Panel::apply(Display &display, const PanelBus &bus) {
    uint32_t *pixels = display.pixels();
    // SDLE = Hsync
    // GDSP = ~ Vsync
    // SDCE0 = ~ DE
    uint8_t hs = bus.sdle;
    uint8_t vs = !bus.gdsp;
    uint8_t de = !bus.sdce0;

    if (!last_hs && hs) {
        x_counter = 0;
        if (vs) {
            y_counter = 0;
            line_valid = false;
            render(display);
        } else {
            if (line_valid)
                y_counter++;
        }
    }

    if (de) {
        if (dbg) {
            set_pixel(
                pixels, x_counter++, y_counter, (bus.dbg_wvfm_tgt >> 12) & 0xf);
            set_pixel(
                pixels, x_counter++, y_counter, (bus.dbg_wvfm_tgt >> 8) & 0xf);
            set_pixel(
                pixels, x_counter++, y_counter, (bus.dbg_wvfm_tgt >> 4) & 0xf);
            set_pixel(
                pixels, x_counter++, y_counter, (bus.dbg_wvfm_tgt >> 0) & 0xf);
        } else {
            set_pixel(pixels, x_counter++, y_counter, (bus.sd >> 6) & 0x3);
            set_pixel(pixels, x_counter++, y_counter, (bus.sd >> 4) & 0x3);
            set_pixel(pixels, x_counter++, y_counter, (bus.sd >> 2) & 0x3);
            set_pixel(pixels, x_counter++, y_counter, (bus.sd >> 0) & 0x3);
        }
        line_valid = true;
    }

    last_hs = hs;
}
