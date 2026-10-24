# Performance

## Cycles per frame

Caster takes 4 pixels per clock, so a frame needs about one cycle for
every 4 pixels, plus the blanking the firmware sets at startup:

| Part | Register | Cycles |
| --- | --- | --- |
| Pixels in a line | `H_ACT` | width / 4 |
| Blanking after each line | `H_FP + H_SYNC + H_BP` | 3 + 1 + 2 = 6 |
| Lines with image | `V_ACT` | height |
| Blanking lines after each frame | `V_FP + V_SYNC + V_BP` | 3 + 1 + 2 = 6 |

```
cycles per frame ≈ (width / 4 + 6) × (height + 6)
```

The blanking is fixed, so the cost grows almost linearly with the number
of pixels: doubling both width and height takes about 4 times the cycles.

## Simulation speed

The speed barely depends on the resolution, because every cycle does
the same work: 4 panel pixels, 4 input pixels and one VRAM access. So
the frame rate is the speed divided by the cycles per frame.

Measured headless with 50 million cycles, with both the testbench and
the Verilator model built with `-O3`:

```sh
time ./build/sim --headless -r 640x480 -c 50000000
```

| Resolution | Cycles per frame | M5 MacBook Air | Ryzen AI 7 350 |
| --- | --- | --- | --- |
| 640 × 480 | ~81 000 | 7.1 MHz, ~88 fps | 5.1 MHz, ~63 fps |
| 1024 × 768 | ~203 000 | 7.2 MHz, ~35 fps | 5.1 MHz, ~25 fps |
| 1280 × 960 | ~315 000 | 7.1 MHz, ~23 fps | 5.1 MHz, ~16 fps |

The simulator runs on a single core, so its speed depends mostly on the
frequency of that core.

## Build options

| Option | Effect on an M5 MacBook Air |
| --- | --- |
| Testbench with `-Og` instead of `-O3` | about half the speed, the model buffers are not inlined |
| Model with `OPT_FAST="-O3"` instead of the default `-Os` | about 1.5 times faster |
| Model with `--threads 2` | about 9 times slower |

Verilator threads do not help here: the design is small, and the
threads have to sync on every `eval()`, three times per cycle, which
costs more than the work they share.
