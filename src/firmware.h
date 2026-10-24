#ifndef FIRMWARE_H
#define FIRMWARE_H

/*
 * Fake MCU firmware. Configures Caster over SPI like the real firmware does
 * on the board.
 */
#include <cstdint>
#include <utility>

class Firmware {
  public:
    /* show_osd: draw the OSD test pattern, off by default */
    explicit Firmware(std::pair<uint64_t, uint64_t> res, bool show_osd);

    /* Run the next step of the firmware, called once per clock cycle. */
    void tick();

  private:
    enum class Step {
        CONFIGURE,
        WAIT_SPI,
        DONE,
    };
    Step step = Step::CONFIGURE;
    bool show_osd;
    std::pair<uint64_t, uint64_t> res;
};

#endif
