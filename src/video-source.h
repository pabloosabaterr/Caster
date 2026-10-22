#ifndef VIDEO_SOURCE_H
#define VIDEO_SOURCE_H

#include "panel.h"
#include <cstdint>
#include <string_view>
#include <unordered_map>

enum class SourceType {
    BOX,
    FIXED_BORDER,
    FIXED_COLOR,
    FIXED_SWAP,
    BLINK,
    IMAGE,
};

inline const std::unordered_map<std::string_view, SourceType> SOURCE_TYPES = {
    {"box", SourceType::BOX},
    {"fixed-border", SourceType::FIXED_BORDER},
    {"fixed-color", SourceType::FIXED_COLOR},
    {"fixed-swap", SourceType::FIXED_SWAP},
    {"blink", SourceType::BLINK},
    {"image", SourceType::IMAGE},
};

/* Video input of Caster. */
struct VideoBus {
    /* From Caster */
    uint8_t ready = 0;

    /* To Caster */
    uint8_t vsync = 0;
    uint32_t pixel = 0;
    uint8_t valid = 0;
};

class VideoSource {
  public:
    explicit VideoSource(SourceType source, PanelType panel);

    void reset();
    void apply(VideoBus &bus);

  private:
    void next_frame();
    uint8_t rgb2y(uint32_t r, uint32_t g, uint32_t b, int x, int y);

    void source_box();
    void source_fixed_border();
    void source_fixed_color();
    void source_fixed_swap();
    void source_blink();
    void source_image();

    uint8_t pixels[DISP_WIDTH * DISP_HEIGHT];
    int x_counter = 0;
    int y_counter = 0;
    int frame_counter = 0;
    bool box_loaded = false;
    uint8_t box_texture[20 * 20];
    int box_x = 0;
    int box_y = 0;
    int box_dir = 0;
    PanelType panel;
    SourceType source;
};

#endif
