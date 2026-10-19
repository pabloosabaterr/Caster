# Known issues

> [!NOTE]
> Issues are never removed from this list. Fixed ones are checked off, so the
> list also works as a record of what differs from the original repository.

## Runtime

- [ ] `./sim` segfaults if `texture.jpg` is missing. The image is not included
      in the repository and the `stbi_load` result is not checked in
      `srcsim.cpp`.

## Build

- [ ] `rtl.mk` calls `nproc`, which does not exist on macOS
      (`nproc: command not found`).
- [ ] `dispsim.cpp:114`: `dispsim_apply` ignores the `gdoe`, `gdclk`, `sdoe`
      and `dbg` panel signals, so the display model does not check them.

## Testbench warnings

- [ ] `intapi.cpp:31-32`: unused variables `last_update` and
      `last_update_duration`.
- [ ] `srcsim.cpp:46`: unused function `rgb2y`.
- [ ] `vramsim.cpp:44`: unused function `print_pixel`.

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
