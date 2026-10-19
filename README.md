# Caster simulator

> [!WARNING]
> The simulator does not build natively on Windows. Use
> [WSL](https://learn.microsoft.com/windows/wsl/install) and follow the Linux
> instructions.

## Getting started

To make it work you'll need:
- [Verilator](https://verilator.org) (tested with 5.052)
- SDL2
- A C++17 compiler and GNU Make
- The `rtl/mulib` submodule:

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

Then build and run from `sim/`:

```sh
make
./sim
```

`make DEVELOPER=1` builds with `-Wextra -Wpedantic -Werror`.

Issues found so far are tracked in [KnownIssues.md](KnownIssues.md).
