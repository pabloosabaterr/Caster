# Caster simulator

> [!NOTE]
> Cycle-accurate simulator of the [Caster](https://gitlab.com/zephray/caster)
> electrophoretic display controller, originally part of the Caster repository.
> Maintained by [@pabloosabaterr](https://github.com/pabloosabaterr) and
> [@pacoalbacetec](https://github.com/pacoalbacetec).

> [!WARNING]
> The simulator does not build natively on Windows. Use
> [WSL](https://learn.microsoft.com/windows/wsl/install) and follow the Linux
> instructions.

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

`./build/sim` expects a `texture.jpg` of exactly 20 x 20 pixels in the
current directory.

The `sim/` folder inside the `caster` submodule is the old copy of this
simulator and is not used.

Issues found so far are tracked in [KnownIssues.md](KnownIssues.md).

## License

The simulator is licensed under MIT, see [LICENSE](LICENSE). The RTL in the
`caster` submodule is licensed under CERN-OHL-P v2. `noise.mem` is converted
from [Moments in Graphics](http://momentsingraphics.de/BlueNoise.html) and is
in the public domain.
