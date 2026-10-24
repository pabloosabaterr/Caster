#ifndef SIM_H
#define SIM_H

#include "digest.h"
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
        bool digest = false;
        std::pair<uint64_t, uint64_t> res = {320, 240};
    };

    explicit Sim(const Config &config);
    ~Sim();

    /* Print the digest of the frame still in progress. */
    void finish_digest();

    /* Hold the core in reset and reset every model. */
    void reset(Display &display);

    /* Run one clock cycle. */
    void tick(Display &display);

    uint64_t cycles() const {
        return tickcount;
    }

    uint64_t panel_frames() const {
        return panel->frame_count();
    }

  private:
    Vcaster *core = nullptr;
    Firmware *firmware = nullptr;
    VideoSource *video_source = nullptr;
    Panel *panel = nullptr;
    Vram *vram = nullptr;
    uint64_t tickcount = 0;
    VerilatedVcdC *trace = nullptr;
    std::optional<Digest> digest;
};

#endif
