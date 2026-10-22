#ifndef VIDEO_SOURCE_H
#define VIDEO_SOURCE_H

#include <cstdint>
#include <string_view>

void srcsim_reset(std::string_view source);

/* Video input of Caster. */
struct VideoBus {
    /* From Caster */
    uint8_t ready = 0;

    /* To Caster */
    uint8_t vsync = 0;
    uint32_t pixel = 0;
    uint8_t valid = 0;
};

void srcsim_apply(VideoBus &bus, std::string_view source);

#endif
