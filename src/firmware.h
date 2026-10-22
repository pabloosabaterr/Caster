#ifndef FIRMWARE_H
#define FIRMWARE_H

/*
 * Fake MCU firmware. Configures Caster over SPI like the real firmware does
 * on the board.
 */
class Firmware {
  public:
    /* Run the next step of the firmware, called once per clock cycle. */
    void tick();

  private:
    enum class Step {
        CONFIGURE,
        WAIT_SPI,
        DONE,
    };
    Step step = Step::CONFIGURE;
};

#endif
