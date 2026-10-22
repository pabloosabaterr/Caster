#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "int-api.h"
#include "spi.h"

static UPDATE_MODE current_mode;
static uint8_t waveform_frames;

static uint8_t get_update_frames(void) {
    // Should be worst case time to clear/ update a frame
    // uint8_t min_time = 10; // Minimum time for non-LUT modes
    // actually, just always return 1s
    return 60;
}

static void wait(void) {
    // Reading is not implemented in the simulator
}

void intapi_init(void) {
    current_mode = UM_AUTO_LUT_NO_DITHER; // Need to sync with the RTL code
    waveform_frames = 38;                 // Need to sync with the RTL code
}

void intapi_load_waveform(uint8_t *waveform, uint8_t frames) {
    wait();
    spi_write_reg8(CSR_LUT_FRAME, 0); // Reset value before loading
    spi_write_reg16(CSR_LUT_ADDR, 0);
    spi_write_bulk(CSR_LUT_WR, waveform, WAVEFORM_SIZE);
    waveform_frames = frames;
}

void intapi_redraw(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    wait();
    spi_write_reg16(CSR_OP_LEFT, x0);
    spi_write_reg16(CSR_OP_TOP, y0);
    spi_write_reg16(CSR_OP_RIGHT, x1);
    spi_write_reg16(CSR_OP_BOTTOM, y1);
    spi_write_reg8(CSR_OP_LENGTH, get_update_frames());
    spi_write_reg8(CSR_OP_CMD, OP_EXT_REDRAW);
}

void intapi_setmode(
    uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, UPDATE_MODE mode) {
    wait();
    spi_write_reg16(CSR_OP_LEFT, x0);
    spi_write_reg16(CSR_OP_TOP, y0);
    spi_write_reg16(CSR_OP_RIGHT, x1);
    spi_write_reg16(CSR_OP_BOTTOM, y1);
    spi_write_reg8(CSR_OP_LENGTH, get_update_frames());
    spi_write_reg8(CSR_OP_PARAM, (uint8_t)mode);
    spi_write_reg8(CSR_OP_CMD, OP_EXT_SETMODE);
}
