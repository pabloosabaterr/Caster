#include "sim.h"
#include "display.h"
#include "firmware.h"
#include "panel.h"
#include "spi.h"
#include "video-source.h"
#include "vram.h"

#include <Vcaster.h>
#include <cstdint>

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

Sim::Sim() {
    core = new Vcaster;
    Verilated::traceEverOn(true);
    firmware = new Firmware;
#ifdef TRACE
    trace = new VerilatedVcdC;
    core->trace(trace, 99);
    trace->open("trace.vcd");
#endif
}

Sim::~Sim() {
#ifdef TRACE
    trace->close();
    delete trace;
#endif
    delete core;
    delete firmware;
}

void Sim::reset(Display &display) {
    core->rst = 1;
    tick(display);
    tick(display);
    core->rst = 0;
    dispsim_reset();
    srcsim_reset();
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

void Sim::tick(Display &display) {
    CoreInputs in{};

    /* Call simulated modules */
    dispsim_apply(display, read_panel_bus(*core));

    VideoBus video = read_video_bus(*core);
    srcsim_apply(video);

    VramBus vram = read_vram_bus(*core);
    vramsim_apply(vram);

    spisim_apply(in.spi_cs, in.spi_sck, in.spi_mosi, core->spi_miso);

    /* Posedge */
    core->clk = 1;
    core->eval();

    /* Apply changed input signals after clock edge */
    core->vin_vsync = video.vsync;
    core->vin_pixel = video.pixel;
    core->vin_valid = video.valid;
    core->bi_pixel = vram.bi_pixel;
    core->bi_valid = vram.bi_valid;
    core->spi_cs = in.spi_cs;
    core->spi_sck = in.spi_sck;
    core->spi_mosi = in.spi_mosi;

    /* Let combinational changes propagate */
    core->eval();
#ifdef TRACE
    trace->dump(tickcount * 10);
#endif

    /* Negedge */
    core->clk = 0;
    core->eval();
#ifdef TRACE
    trace->dump(tickcount * 10 + 5);
#endif
    tickcount++;

    firmware->tick();
}
