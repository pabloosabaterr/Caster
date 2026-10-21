# Known issues

> [!NOTE]
> Issues are never removed from this list. Fixed ones are checked off, so the
> list also works as a record of what differs from the original repository.

## Runtime

- [x] `./sim` segfaults if `texture.jpg` is missing. The image is not included
      in the repository and the `stbi_load` result is not checked in
      `srcsim.cpp`.
- [x] `srcsim.cpp:141`: `texture.jpg` must be exactly 20 x 20. Other sizes
      give a corrupted sprite, and images under 400 pixels are read out of
      bounds.
- [x] The screen ends up fully black. `testmain()` never loads the tone LUT,
      which starts zeroed and maps every input pixel to black. `intapi.h` was
      also missing registers 48 to 51 from `rtl/defines.vh`.
- [x] The OSD enabled in `testmain()` covers 320 x 200 of the 320 x 240
      screen, hiding most of the input image.
- [ ] `noise.mem` and `default_waveform.mem` are referenced by bare file name
      in the RTL, so `$readmem` only finds them in the current directory. As
      a workaround, a copy of `noise.mem` is kept in the repository root until
      configurable paths are merged in Caster.
- [x] `TRACE` is defined by default in `main.cpp`, so every run writes all
      signals to `trace.vcd`, which grows fast and slows the simulation down.
- [x] `spisim.cpp:63`: prints every received SPI byte, flooding the terminal
      during initialization.
- [ ] Options (input pattern, OSD, trace, cycle limit) are set with `#define`
      and `#if 1` / `#elif 0` chains, so changing them needs a rebuild and the
      disabled code is never compiled.
- [x] `tick()` passed uninitialized local variables to the core when a model
      did not set them, so two identical runs gave different traces.

## Build

- [x] `rtl.mk` calls `nproc`, which does not exist on macOS
      (`nproc: command not found`).
- [ ] Changing a `.v` file does not rebuild the RTL, the `Vcaster__ALL.a`
      rule has no prerequisites. A `make clean` is needed.
- [ ] Changing a `.h` file does not rebuild the `.cpp` files that include it,
      no dependency files are generated.
- [ ] The `all` target relinks on every `make`, even with nothing changed.
- [ ] Sub-makes are called with `make` instead of `$(MAKE)`, so flags like
      `-j` are not passed down.

## Testbench warnings

- [x] `intapi.cpp:31-32`: unused variables `last_update` and
      `last_update_duration`.
- [x] `srcsim.cpp:46`: unused function `rgb2y`.
- [x] `vramsim.cpp:44`: unused function `print_pixel`.
- [x] `dispsim.cpp:114`: `dispsim_apply` ignores the `gdoe`, `gdclk`, `sdoe`
      and `dbg` panel signals, so the display model does not check them.

## RTL lint warnings

- [ ] `osd_addr.v`: file name does not match module name `osd_overlay_expand`
      (DECLFILENAME).
- [ ] `line_reverse.v:1`: missing timescale (TIMESCALEMOD).
- [ ] `line_reverse.v:30,32`: unnamed generate blocks (GENUNNAMED).
- [ ] `caster.v:448,908`: unused signals `scan_in_vbp` and `clk_en`
      (UNUSEDSIGNAL).
- [ ] `caster.v:538,645`, `osd_addr.v:8`, `pixel_processing.v:252`,
      `blue_noise_dithering.v:23`: partially or fully unused signals
      (UNUSEDSIGNAL).
- [ ] `blue_noise_dithering.v:40-43`: `addr_lo_0` to `addr_lo_3` are never
      driven (UNDRIVEN).
- [ ] `blue_noise_dithering.v:78`: `always @(*)` block never executes
      (ALWNEVER). Together with the previous item, looks like an unfinished
      part of the blue noise dithering.
- [ ] `caster.v:20`: `rst` is used both as synchronous and asynchronous reset
      (SYNCASYNCNET). Affects synthesis, needs confirmation from upstream
      before changing.
