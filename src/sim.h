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
    struct Config {
        bool enable_trace = false;
        bool show_osd = false;
        SourceType source_type = SourceType::BOX;
        PanelType panel_type = PanelType::MONOCHROME;
        std::optional<std::pair<uint64_t, uint64_t>> watch;
        bool panel_debug = false;
    };

    explicit Sim(const Config &config);
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
