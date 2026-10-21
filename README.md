# Caster simulator

> [!NOTE]
> Cycle-accurate simulator of the [Caster](https://gitlab.com/zephray/caster)
> electrophoretic display controller. Originally written by Wenting Zhang as
> part of the Caster repository.
> Maintained by [@pabloosabaterr](https://github.com/pabloosabaterr) and
> [@pacoalbacetec](https://github.com/pacoalbacetec).

> [!WARNING]
> The simulator does not build natively on Windows. Use
> [WSL](https://learn.microsoft.com/windows/wsl/install) and follow the Linux
> instructions.

The idea behind this project is to make e-ink development more accessible for
anyone. Currently the monetary gap to start working on such technology is
usually big, this means that people that want but can't afford to buy the
hardware are left out.

For that we want to develop a proper simulator opening the doors for anyone to
contribute to this technology that is each day better and more relevant.

> [!TIP]
> If you want to contribute, check out [CONTRIBUTING.md](CONTRIBUTING.md)

Any kind of help is welcome. We, Paco and Pablo, are two CS & Eng. students
doing this in our free time, so the development will not be very rushed but
continued.

Feel free to reach us through email (pablo@pablosabater.dev,
pacoalbacete06@gmail.com) or through the [Modos](https://www.modos.tech)
[Discord](https://discord.com/invite/6ktE6VxSyh).

## Getting started

To make it work you'll need:
- [Verilator](https://verilator.org) (tested with 5.052)
- SDL2
- A C++17 compiler and GNU Make
- The `caster` submodule, which contains the RTL:

```sh
  git submodule update --init --recursive
```

On macOS:

```sh
brew install verilator sdl2
```

On Debian / Ubuntu:

```sh
sudo apt install verilator libsdl2-dev build-essential
```

Then build and run from the repository root:

```sh
make
./build/sim
```

`make DEVELOPER=1` builds with `-Wextra -Wpedantic -Werror`.

## Caveats

The `sim/` folder inside the `caster` submodule is the old copy of this
simulator and is not used.

Issues found so far are tracked in [KnownIssues.md](KnownIssues.md).

A main roadmap has not yet been done.

## License

The simulator is licensed under MIT, see [LICENSE](LICENSE). The RTL in the
`caster` submodule is licensed under CERN-OHL-P v2. `noise.mem` is converted
from [Moments in Graphics](http://momentsingraphics.de/BlueNoise.html) and is
in the public domain.
