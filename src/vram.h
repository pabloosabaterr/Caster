#ifndef VRAM_H
#define VRAM_H

#include "panel.h"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

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

class Vram {
  public:
    explicit Vram(std::optional<std::pair<uint64_t, uint64_t>> watch);

    void apply(VramBus &bus);
    void reset();

  private:
    /* 16 bits of state per pixel, 4 pixels per 64-bit word */
    static constexpr std::size_t VRAM_SIZE = DISP_WIDTH * DISP_HEIGHT / 4;

    uint64_t vram[VRAM_SIZE];
    std::size_t rdptr = 0;
    std::size_t wrptr = 0;
    uint8_t last_trigger = 0;
    std::optional<std::size_t> watch_word;
};

#endif
