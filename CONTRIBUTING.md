# Contributing

Any kind of help is welcome. This document explains how we work. If something
here is unclear or you disagree with a rule, open an issue or reach us through
the contacts in the [README](README.md).

The rules for the code itself are in
[CODING_GUIDELINES.md](CODING_GUIDELINES.md).

## Workflow

1. Fork the repository and create a branch for each change:
   `git switch -c fix-texture-size`.
2. Build in developer mode, it must build with no warnings:
   `make clean && make DEVELOPER=1`.
3. Push the branch and open a pull request against `main`. The CI must pass
   before merging.
4. Pull requests are merged with “Rebase and merge”, so every commit lands
   in `main` as it is. Keep them clean.

## Commits

- Each commit should be a logical unit that is easy to follow on its own.
  Keeping refactors and bug fixes in separate commits is preferred, but not
  required.
- Title as `<main file changed>: <short description>`, in the imperative and
  under 72 characters. Leave out the file extension:

  ```
  srcsim: check the texture size
  Makefile: rebuild the RTL when a .v file changes
  README: add Windows instructions
  ```

- A body is not required, but it is very much appreciated. Explain why the
  change is needed, not only what it does, and wrap it at 72 columns.
- Sign off every commit with `git commit -s`. It certifies that you wrote the
  change or have the right to submit it under the project license. If you
  forgot, `git rebase --signoff main` signs off all the commits of your
  branch.

## RTL changes

The RTL comes from the `caster` submodule and is shared with the real
hardware. Do not change it here. Propose RTL changes as merge requests to
[Caster](https://gitlab.com/zephray/caster).
