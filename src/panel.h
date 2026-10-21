#pragma once

#include <cstdint>

/* #define K3 */
#define MONO

class Display;

constexpr int DISP_WIDTH = 320;
constexpr int DISP_HEIGHT = 240;

void dispsim_reset();

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

void dispsim_apply(Display &display, const PanelBus &bus);
