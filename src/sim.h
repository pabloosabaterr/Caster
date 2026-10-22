#ifndef SIM_H
#define SIM_H

#include "panel.h"
#include "video-source.h"
#include <cstdint>
#include <optional>
#include <utility>

class Vcaster;
class VerilatedVcdC;
class Display;
class Firmware;
class VideoSource;
class Vram;

/* The Caster core, its trace and the clock that drives it. */
class Sim {
  public:
    explicit Sim(bool enable_trace,
                 bool show_osd,
                 SourceType source,
                 PanelType panel,
                 std::optional<std::pair<uint64_t, uint64_t>> watch,
                 bool dbg);
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
    VideoSource *video_source = nullptr;
    Panel *panel = nullptr;
    Vram *vram = nullptr;
    uint64_t tickcount = 0;
    VerilatedVcdC *trace = nullptr;
};

#endif
