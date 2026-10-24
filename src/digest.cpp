#include "digest.h"

#include <cinttypes>
#include <cstdio>

void Digest::apply(const PanelBus &panel, const VramBus &vram, uint64_t cycle) {
    uint8_t hs = panel.sdle;
    uint8_t vs = !panel.gdsp;

    /* A new frame starts on the hs rising edge while vs is set */
    if (!last_hs && hs && vs) {
        print();
        panel_hash.reset();
        vram_hash.reset();
        frame++;
        frame_start = cycle;
    }
    last_hs = hs;

    panel_hash.add(panel.gdoe, 1);
    panel_hash.add(panel.gdclk, 1);
    panel_hash.add(panel.gdsp, 1);
    panel_hash.add(panel.sdle, 1);
    panel_hash.add(panel.sdoe, 1);
    panel_hash.add(panel.sd, 2);
    panel_hash.add(panel.sdce0, 1);

    if (vram.bo_valid)
        vram_hash.add(vram.bo_pixel, 8);
}

void Digest::finish() const {
    print();
}

void Digest::print() const {
    printf("frame %" PRIu64 " cycle %" PRIu64 " panel=%016" PRIx64
           " vram=%016" PRIx64 "\n",
           frame,
           frame_start,
           panel_hash.value,
           vram_hash.value);
}
