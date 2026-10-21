#ifndef SPI_H
#define SPI_H

#include <cstdint>

void spisim_reset();

/* SPI between the MCU and Caster. */
struct SpiBus {
    /* From Caster */
    uint8_t miso = 0;

    /* To Caster, idle high */
    uint8_t cs = 1;
    uint8_t sck = 1;
    uint8_t mosi = 1;
};

void spisim_apply(SpiBus &bus);

// User functions
void spi_write_reg8(uint8_t addr, uint8_t val);
void spi_write_reg16(uint8_t addr, uint16_t val);
void spi_write_bulk(uint8_t addr, uint8_t *buf, int length);
bool spi_is_busy(void);

#endif
