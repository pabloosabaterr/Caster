#ifndef VRAM_H
#define VRAM_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

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
    explicit Vram(std::optional<std::pair<uint64_t, uint64_t>> watch,
                  std::pair<uint64_t, uint64_t> res);

    void apply(VramBus &bus);
    void reset();

  private:
    std::vector<uint64_t> vram;
    std::size_t rdptr = 0;
    std::size_t wrptr = 0;
    uint8_t last_trigger = 0;
    std::optional<std::size_t> watch_word;
};

#endif
