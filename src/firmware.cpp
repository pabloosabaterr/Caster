#include "firmware.h"
#include "int-api.h"
#include "log.h"
#include "panel.h"
#include "spi.h"

static constexpr int OSD_WIDTH = 320;
static constexpr int OSD_HEIGHT = 200;
static constexpr int OSD_BUF_SIZE = OSD_WIDTH * OSD_HEIGHT / 8;

// This function is called every tick
void Firmware::tick() {
    switch (step) {
    case Step::CONFIGURE:
        spi_write_reg8(CSR_CFG_V_FP, 3);
        spi_write_reg8(CSR_CFG_V_SYNC, 1);
        spi_write_reg8(CSR_CFG_V_BP, 2);
        spi_write_reg16(CSR_CFG_V_ACT, DISP_HEIGHT);
        spi_write_reg8(CSR_CFG_H_FP, 3);
        spi_write_reg8(CSR_CFG_H_SYNC, 1);
        spi_write_reg8(CSR_CFG_H_BP, 2);
        spi_write_reg16(CSR_CFG_H_ACT, DISP_WIDTH / 4);
        spi_write_reg8(CSR_CFG_MINDRV, 1);

        if (show_osd) {
            spi_write_reg16(CSR_OSD_LEFT, 0);
            spi_write_reg16(CSR_OSD_RIGHT, OSD_WIDTH / 4);
            spi_write_reg16(CSR_OSD_TOP, 0);
            spi_write_reg16(CSR_OSD_BOTTOM, OSD_HEIGHT);
            spi_write_reg16(CSR_OSD_ADDR, 0);
            for (int i = 0; i < OSD_BUF_SIZE; i++)
                spi_write_reg8(CSR_OSD_WR, 0x55);
            spi_write_reg8(CSR_OSD_EN, 1);
        }

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
            log_info("Initialization over SPI done.\n");
            step = Step::DONE;
        }
        break;
    case Step::DONE:
        break;
    }
}
