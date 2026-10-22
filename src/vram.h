#pragma once

void vramsim_reset();
void vramsim_apply(uint8_t b_trigger,
                   uint64_t &bi_pixel,
                   uint8_t &bi_valid,
                   uint8_t bi_ready,
                   uint64_t bo_pixel,
                   uint8_t bo_valid);
