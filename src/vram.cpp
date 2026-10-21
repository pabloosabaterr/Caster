#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "vram.h"
#include "log.h"

// 320*240*2 = 153.6KB = 19.2Kw
#define VRAM_SIZE 32768
static uint64_t vram[VRAM_SIZE];
static size_t rdptr, wrptr;
static uint8_t last_trigger;

// #define TRIGGER 2070

void vramsim_reset() {
    memset(vram, 0, VRAM_SIZE * sizeof(uint64_t));
    rdptr = 0;
    wrptr = 0;
    last_trigger = 0;
}

[[maybe_unused]]
static void print_pixel(uint16_t pixel) {
    int mode = ((pixel >> 14) & 0x3);
    int fcnt = ((pixel >> 4) & 0x3f);
    int pixl = ((pixel) & 0xf);
    log_debug("Mode: %d, Fcnt: %d, Pixl: %d\n", mode, fcnt, pixl);
}

void vramsim_apply(VramBus &bus) {
    if (!last_trigger && bus.trigger) {
        rdptr = 0;
        wrptr = 0;
    }
    bus.bi_valid = 1;
    if (bus.bi_ready) {
        bus.bi_pixel = vram[rdptr++];
#ifdef TRIGGER
        if (rdptr == TRIGGER) {
            log_debug("Rd\n");
            print_pixel(bus.bi_pixel & 0xffff);
            print_pixel((bus.bi_pixel >> 16) & 0xffff);
            print_pixel((bus.bi_pixel >> 32) & 0xffff);
            print_pixel((bus.bi_pixel >> 48) & 0xffff);
        }
#endif
    }
    if (bus.bo_valid) {
        vram[wrptr++] = bus.bo_pixel;
#ifdef TRIGGER
        if (wrptr == TRIGGER) {
            log_debug("Wr\n");
            print_pixel(bus.bo_pixel & 0xffff);
            print_pixel((bus.bo_pixel >> 16) & 0xffff);
            print_pixel((bus.bo_pixel >> 32) & 0xffff);
            print_pixel((bus.bo_pixel >> 48) & 0xffff);
        }
#endif
    }
    last_trigger = bus.trigger;
}
