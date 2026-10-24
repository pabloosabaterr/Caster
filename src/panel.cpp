#include <cstdint>
#include <stdio.h>

#include "panel.h"
#include "display.h"

#include <SDL.h>

#define ACCEL 20

Panel::Panel(PanelType type, bool dbg, std::pair<uint64_t, uint64_t> res)
    : type(type), dbg(dbg), width(res.first), height(res.second),
      localbuf(width * height), speedbuf(width * height) {
    reset();
}

void Panel::set_pixel(uint32_t *pixels, int x, int y, uint8_t input) {
    int32_t pixel;
    if (x >= width)
        return;
    if (y >= height)
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
        pixel = localbuf[y * width + x];
        int32_t speed = speedbuf[y * width + x];
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
        localbuf[y * width + x] = pixel;
        speedbuf[y * width + x] = speed;
    }

    switch (type) {
    case PanelType::KALEIDO3: {
        int c = (x + (height - y)) % 3;
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
    pixels[y * width + x] = pixel;
}

void Panel::reset() {
    x_counter = 0;
    y_counter = 0;
    last_hs = 0;
    std::fill(localbuf.begin(), localbuf.end(), 0xff);
    line_valid = false;
    std::fill(speedbuf.begin(), speedbuf.end(), 0);
}

void Panel::render(Display &display) {
    if (display.is_headless())
        return;

    display.present();
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
            frame_count_++;
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
