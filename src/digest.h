#ifndef DIGEST_H
#define DIGEST_H

#include "panel.h"
#include "vram.h"
#include <cstdint>

/*
 * FNV-1a 64-bit hash. Not cryptographic, only used to detect any change
 * in the simulator output between runs.
 */
struct Fnv1a {
    static constexpr uint64_t OFFSET = 14695981039346656037ull;
    static constexpr uint64_t PRIME = 1099511628211ull;

    uint64_t value = OFFSET;

    /* Mix the lowest `bytes` bytes of data, least significant first. */
    void add(uint64_t data, int bytes) {
        for (int i = 0; i < bytes; i++) {
            value ^= data & 0xff;
            value *= PRIME;
            data >>= 8;
        }
    }

    void reset() {
        value = OFFSET;
    }
};

/*
 * Summary of the simulator output for regression tests: one line per
 * frame on stdout with a hash of every panel signal on every cycle and
 * a hash of every word Caster writes to the VRAM.
 */
class Digest {
  public:
    /* Hash the buses of one cycle, printing the frame that just ended. */
    void apply(const PanelBus &panel, const VramBus &vram, uint64_t cycle);

    void finish() const;

  private:
    void print() const;

    Fnv1a panel_hash;
    Fnv1a vram_hash;
    uint64_t frame = 0;
    uint64_t frame_start = 0;
    uint8_t last_hs = 0;
};

#endif
