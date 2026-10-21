# Coding guidelines

Follow these rules in new code.

> [!NOTE]
> Existing code does not follow all these rules yet. In particular, it uses
> `//` comments, `else` on its own line and `#pragma once`. Do not reformat
> existing code only to apply them, fix it as it gets modified for another
> reason.

## Formatting

- 4 spaces for indentation, no tabs. Tabs only in Makefiles.
- 80 columns maximum, in code and in Markdown.
- Opening braces on the same line, for functions and control statements.
- `else` goes on the same line as the closing brace: `} else {`.
- Single-statement bodies go without braces, on their own line. If any branch
  of an `if` / `else` chain needs braces, all branches get them:

```cpp
  if (dir == 0)
      x++;
  else
      x--;

  if (dir == 0) {
      x++;
      y++;
  } else {
      x--;
  }
```

- The pointer asterisk goes next to the name: `uint8_t *data`.
- `.editorconfig` sets most of this up in your editor.

## Naming

| Kind | Style | Example |
|---|---|---|
| Types (classes, structs, enums) | PascalCase | `CoreInputs` |
| Functions, variables, members | snake_case | `srcsim_apply`, `frame_counter` |
| Constants and macros | UPPER_CASE | `DISP_WIDTH`, `SIM_STEP` |
| Files and directories | kebab-case | `disp-sim.cpp`, `third-party/` |
| Markdown documents | UPPER_SNAKE_CASE | `KNOWN_ISSUES.md` |

When declaring a variable, do not repeat `struct`: write `CoreInputs inputs;`,
not `struct CoreInputs inputs;`.

## Language

- C++17.
- Use `constexpr` for constants. Use `#define` only for what the preprocessor
  needs, such as `#ifdef` switches and include guards.
- Every header has an include guard, named after the file in UPPER_CASE:

```cpp
  #ifndef DISP_SIM_H
  #define DISP_SIM_H

  /* ... */

  #endif /* DISP_SIM_H */
```

- Avoid hidden state. Keep state in structs or classes, not in
  function-level `static` variables, so it can be reset and whoever reads the
  code can see it.
- Use fixed-width integer types (`uint8_t`, `uint32_t`...) for values that
  map to RTL signals, matching the signal width.
- Comments in English. Explain why the code does something, not what it does.
- Use `/* */` comments, for both single-line and multi-line comments:

```cpp
  /* Single-line comment */

  /*
   * Multi-line comment, with the asterisks aligned
   * on each line.
   */
```

## Third-party code

Code in `third-party/` is not ours. Do not modify it. If needed, update it by
replacing it with a newer upstream version.
