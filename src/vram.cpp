#include <cstdint>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include "vram.h"
#include "log.h"

Vram::Vram(std::optional<std::pair<uint64_t, uint64_t>> watch) {
    if (watch) {
        auto [x, y] = *watch;
        watch_word = (y * DISP_WIDTH + x) / 4;
    }
    reset();
}

void Vram::reset() {
    memset(vram, 0, VRAM_SIZE * sizeof(uint64_t));
    rdptr = 0;
    wrptr = 0;
    last_trigger = 0;
}

static void print_pixel(uint16_t pixel) {
    int mode = ((pixel >> 14) & 0x3);
    int fcnt = ((pixel >> 4) & 0x3f);
    int pixl = ((pixel) & 0xf);
    log_debug("Mode: %d, Fcnt: %d, Pixl: %d\n", mode, fcnt, pixl);
}

void Vram::apply(VramBus &bus) {
    if (!last_trigger && bus.trigger) {
        rdptr = 0;
        wrptr = 0;
    }
    bus.bi_valid = 1;
    if (bus.bi_ready) {
        if (watch_word && rdptr == *watch_word) {
            log_debug("Rd\n");
            print_pixel(bus.bi_pixel & 0xffff);
            print_pixel((bus.bi_pixel >> 16) & 0xffff);
            print_pixel((bus.bi_pixel >> 32) & 0xffff);
            print_pixel((bus.bi_pixel >> 48) & 0xffff);
        }
        bus.bi_pixel = vram[rdptr++];
    }
    if (bus.bo_valid) {
        if (watch_word && wrptr == *watch_word) {
            log_debug("Wr\n");
            print_pixel(bus.bo_pixel & 0xffff);
            print_pixel((bus.bo_pixel >> 16) & 0xffff);
            print_pixel((bus.bo_pixel >> 32) & 0xffff);
            print_pixel((bus.bo_pixel >> 48) & 0xffff);
        }
        vram[wrptr++] = bus.bo_pixel;
    }
    last_trigger = bus.trigger;
}
