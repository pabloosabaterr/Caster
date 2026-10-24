#ifndef PANEL_H
#define PANEL_H

#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <vector>

class Display;

struct PanelBus {
    uint8_t gdoe = 0;
    uint8_t gdclk = 0;
    uint8_t gdsp = 0;
    uint8_t sdle = 0;
    uint8_t sdoe = 0;
    uint16_t sd = 0;
    uint8_t sdce0 = 0;
    uint16_t dbg_wvfm_tgt = 0;
};

enum class PanelType {
    MONOCHROME,
    KALEIDO3,
};

inline const std::unordered_map<std::string_view, PanelType> PANEL_TYPES = {
    {"mono", PanelType::MONOCHROME},
    {"kaleido3", PanelType::KALEIDO3},
};

class Panel {
  public:
    explicit Panel(PanelType type, bool dbg, std::pair<uint64_t, uint64_t> res);

    void reset();
    void apply(Display &display, const PanelBus &bus);
    uint64_t frame_count() const {
        return frame_count_;
    }

  private:
    void set_pixel(uint32_t *pixels, int x, int y, uint8_t input);
    void render(Display &display);

    PanelType type;
    bool dbg;
    int width;
    int height;

    /* Particle model, one entry per pixel */
    std::vector<uint8_t> localbuf;
    std::vector<int8_t> speedbuf;

    int x_counter = 0;
    int y_counter = 0;
    int last_hs = 0;
    bool line_valid = false;

    uint64_t frame_count_ = 0;
};

#endif
