#ifndef SIM_H
#define SIM_H

#include <cstdint>

class Vcaster;
class VerilatedVcdC;
class Display;
class Firmware;

/* The Caster core, its trace and the clock that drives it. */
class Sim {
  public:
    Sim();
    ~Sim();

    /* Hold the core in reset and reset every model. */
    void reset(Display &display);

    /* Run one clock cycle. */
    void tick(Display &display);

    uint64_t cycles() const {
        return tickcount;
    }

  private:
    Vcaster *core = nullptr;
    Firmware *firmware = nullptr;
    uint64_t tickcount = 0;
    /* TODO: Will wait until parse-args enables the --trace option */
    [[maybe_unused]] VerilatedVcdC *trace = nullptr;
};

#endif
