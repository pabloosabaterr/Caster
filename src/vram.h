#ifndef VRAM_H
#define VRAM_H

#include <cstdint>

void vramsim_reset();

/* Bus between Caster and the external VRAM. */
struct VramBus {
    /* From Caster */
    uint8_t trigger = 0;
    uint8_t bi_ready = 0;
    uint64_t bo_pixel = 0;
    uint8_t bo_valid = 0;

    /* To Caster */
    uint64_t bi_pixel = 0;
    uint8_t bi_valid = 0;
};

void vramsim_apply(VramBus &bus);

#endif
