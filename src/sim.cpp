#include "sim.h"
#include "display.h"
#include "firmware.h"
#include "panel.h"
#include "spi.h"
#include "video-source.h"
#include "vram.h"

#include <Vcaster.h>
#include <cstdint>
#include <string_view>
#include "verilated_vcd_c.h"

/* Legacy function required by some platforms/ versions of Verilator */
double sc_time_stamp() {
    return 0;
}

struct CoreInputs {
    // Video input
    uint8_t vin_vsync = 0;
    uint32_t vin_pixel = 0;
    uint8_t vin_valid = 0;

    // VRAM read
    uint64_t bi_pixel = 0;
    uint8_t bi_valid = 0;

    // SPI
    uint8_t spi_cs = 1;
    uint8_t spi_sck = 1;
    uint8_t spi_mosi = 1;
};

Sim::Sim(bool enable_trace, bool show_osd, std::string_view source) {
    core = new Vcaster;
    Verilated::traceEverOn(true);
    firmware = new Firmware(show_osd);
    video_source = new VideoSource(source);
    if (enable_trace) {
        trace = new VerilatedVcdC;
        core->trace(trace, 99);
        trace->open("trace.vcd");
    }
}

Sim::~Sim() {
    if (trace) {
        trace->close();
        delete trace;
    }
    delete core;
    delete firmware;
}

void Sim::reset(Display &display) {
    core->rst = 1;
    tick(display);
    tick(display);
    core->rst = 0;
    dispsim_reset();
    video_source->reset();
    vramsim_reset();
    spisim_reset();
    core->sys_ready = 1;
}

static PanelBus read_panel_bus(const Vcaster &core) {
    PanelBus bus;
    bus.gdoe = core.epd_gdoe;
    bus.gdclk = core.epd_gdclk;
    bus.gdsp = core.epd_gdsp;
    bus.sdle = core.epd_sdle;
    bus.sdoe = core.epd_sdoe;
    bus.sd = core.epd_sd;
    bus.sdce0 = core.epd_sdce0;
    bus.dbg_wvfm_tgt = core.dbg_wvfm_tgt;
    return bus;
}

static VideoBus read_video_bus(const Vcaster &core) {
    VideoBus bus;
    bus.ready = core.vin_ready;
    return bus;
}

static VramBus read_vram_bus(const Vcaster &core) {
    VramBus bus;
    bus.trigger = core.b_trigger;
    bus.bi_ready = core.bi_ready;
    bus.bo_pixel = core.bo_pixel;
    bus.bo_valid = core.bo_valid;
    return bus;
}

static SpiBus read_spi_bus(const Vcaster &core) {
    SpiBus bus;
    bus.miso = core.spi_miso;
    return bus;
}

void Sim::tick(Display &display) {
    /* Call simulated modules */
    dispsim_apply(display, read_panel_bus(*core));

    VideoBus video = read_video_bus(*core);
    video_source->apply(video);

    VramBus vram = read_vram_bus(*core);
    vramsim_apply(vram);

    SpiBus spi = read_spi_bus(*core);
    spisim_apply(spi);

    /* Posedge */
    core->clk = 1;
    core->eval();

    /* Apply changed input signals after clock edge */
    core->vin_vsync = video.vsync;
    core->vin_pixel = video.pixel;
    core->vin_valid = video.valid;
    core->bi_pixel = vram.bi_pixel;
    core->bi_valid = vram.bi_valid;
    core->spi_cs = spi.cs;
    core->spi_sck = spi.sck;
    core->spi_mosi = spi.mosi;

    /* Let combinational changes propagate */
    core->eval();
    if (trace)
        trace->dump(tickcount * 10);

    /* Negedge */
    core->clk = 0;
    core->eval();
    if (trace)
        trace->dump(tickcount * 10 + 5);

    tickcount++;

    firmware->tick();
}
