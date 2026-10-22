#pragma once

void spisim_reset();
void spisim_apply(uint8_t &spi_cs,
                  uint8_t &spi_sck,
                  uint8_t &spi_mosi,
                  const uint8_t spi_miso);
// User functions
void spi_write_reg8(uint8_t addr, uint8_t val);
void spi_write_reg16(uint8_t addr, uint16_t val);
void spi_write_bulk(uint8_t addr, uint8_t *buf, int length);
bool spi_is_busy(void);
