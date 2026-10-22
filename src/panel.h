#ifndef PANEL_H
#define PANEL_H

#include <cstdint>
#include <string_view>
#include <unordered_map>

class Display;

constexpr int DISP_WIDTH = 320;
constexpr int DISP_HEIGHT = 240;

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
    explicit Panel(PanelType type, bool dbg);

    void reset();
    void apply(Display &display, const PanelBus &bus);

  private:
    void set_pixel(uint32_t *pixels, int x, int y, uint8_t input);
    void render(Display &display);

    PanelType type;
    bool dbg;

    uint8_t localbuf[DISP_WIDTH * DISP_HEIGHT];
    int8_t speedbuf[DISP_WIDTH * DISP_HEIGHT];

    int x_counter = 0;
    int y_counter = 0;
    int last_hs = 0;
    bool line_valid = false;

    float time_delta = 0.0f;
    uint32_t last_ticks = 0;
};

#endif
