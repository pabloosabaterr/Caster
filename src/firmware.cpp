/*
 * Caster simulator
 * Copyright 2021 Wenting Zhang
 * Copyright 2026 Pablo Sabater
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "firmware.h"
#include "int-api.h"
#include "spi.h"

#include <cstdio>

static constexpr int OSD_WIDTH = 320;
static constexpr int OSD_HEIGHT = 200;
static constexpr int OSD_BUF_SIZE = OSD_WIDTH * OSD_HEIGHT / 8;

void Firmware::tick() {
    switch (step) {
    case Step::CONFIGURE:
        spi_write_reg8(CSR_CFG_V_FP, 3);
        spi_write_reg8(CSR_CFG_V_SYNC, 1);
        spi_write_reg8(CSR_CFG_V_BP, 2);
        spi_write_reg16(CSR_CFG_V_ACT, 240);
        spi_write_reg8(CSR_CFG_H_FP, 3);
        spi_write_reg8(CSR_CFG_H_SYNC, 1);
        spi_write_reg8(CSR_CFG_H_BP, 2);
        spi_write_reg16(CSR_CFG_H_ACT, 80);
        spi_write_reg8(CSR_CFG_MINDRV, 1);

        // Enable OSD
        spi_write_reg16(CSR_OSD_LEFT, 0);
        spi_write_reg16(CSR_OSD_RIGHT, OSD_WIDTH / 4);
        spi_write_reg16(CSR_OSD_TOP, 0);
        spi_write_reg16(CSR_OSD_BOTTOM, OSD_HEIGHT);
        spi_write_reg16(CSR_OSD_ADDR, 0);
        for (int i = 0; i < OSD_BUF_SIZE; i++) {
            spi_write_reg8(CSR_OSD_WR, 0x55);
        }
        spi_write_reg8(CSR_OSD_EN, 1);
        spi_write_reg8(CSR_CFG_MIRROR, 1);

        spi_write_reg8(CSR_TONE_ADDR, 0);
        for (int i = 0; i < 256; i++)
            spi_write_reg8(CSR_TONE_WR, i);

        spi_write_reg8(CSR_ENABLE, 1); // Enable refresh
        // Read back status
        spi_write_reg8(CSR_STATUS, 0);
        step = Step::WAIT_SPI;
        break;
    case Step::WAIT_SPI:
        if (!spi_is_busy()) {
            printf("Initialization over SPI done.\n");
            step = Step::DONE;
        }
        break;
    case Step::DONE:
        break;
    }
}
