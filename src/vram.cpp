#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "vram.h"

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
    printf("Mode: %d, Fcnt: %d, Pixl: %d\n", mode, fcnt, pixl);
}

void vramsim_apply(uint8_t b_trigger,
                   uint64_t &bi_pixel,
                   uint8_t &bi_valid,
                   uint8_t bi_ready,
                   uint64_t bo_pixel,
                   uint8_t bo_valid) {
    if (!last_trigger && b_trigger) {
        rdptr = 0;
        wrptr = 0;
    }
    bi_valid = 1;
    if (bi_ready) {
        bi_pixel = vram[rdptr++];
#ifdef TRIGGER
        if (rdptr == TRIGGER) {
            printf("Rd\n");
            print_pixel(bi_pixel & 0xffff);
            print_pixel((bi_pixel >> 16) & 0xffff);
            print_pixel((bi_pixel >> 32) & 0xffff);
            print_pixel((bi_pixel >> 48) & 0xffff);
        }
#endif
    }
    if (bo_valid) {
        vram[wrptr++] = bo_pixel;
#ifdef TRIGGER
        if (wrptr == TRIGGER) {
            printf("Wr\n");
            print_pixel(bo_pixel & 0xffff);
            print_pixel((bo_pixel >> 16) & 0xffff);
            print_pixel((bo_pixel >> 32) & 0xffff);
            print_pixel((bo_pixel >> 48) & 0xffff);
        }
#endif
    }
    last_trigger = b_trigger;
}
