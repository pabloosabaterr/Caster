#pragma once

#include <cstdint>

/* #define K3 */
#define MONO

class Display;

constexpr int DISP_WIDTH = 320;
constexpr int DISP_HEIGHT = 240;

void dispsim_reset();
void dispsim_apply(Display &display,
                   const uint8_t gdoe,
                   const uint8_t gdclk,
                   const uint8_t gdsp,
                   const uint8_t sdle,
                   const uint8_t sdoe,
                   const uint8_t sd,
                   const uint8_t sdce0,
                   const uint16_t dbg);
