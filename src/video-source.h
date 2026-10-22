#pragma once

void srcsim_reset();
void srcsim_apply(uint8_t &vsync,
                  uint32_t &pixel,
                  uint8_t &valid,
                  const uint8_t ready);
