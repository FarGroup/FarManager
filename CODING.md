# Coding conventions

The code in the repository is the source of truth: match the surrounding code
first. This file collects the conventions that are easy to get wrong.

## Source file layout

### Include block

Keep the include block structured with the standard section comments, in this
order:

1. `// BUGBUG` — `#include "platform.headers.hpp"`, for the files that need the
   Windows headers pulled in first. This is a known wart, not a pattern to copy.
2. `// Self` — the file's own header (`.cpp` files only; headers have no `Self`).
3. `// Internal` — other Far core headers.
4. `// Platform` — the `platform.*` OS abstraction headers.
5. `// Common` — `common.hpp` first, then the `common/...` utility headers
   (`common/from_string.hpp`, `common/scope_exit.hpp`, ...).
6. `// External` — third-party libraries and their Far wrappers (`format.hpp`
   for fmt, `tinyxml.hpp`, `testing.hpp`).

Keep the section comments even when a section is empty, and separate the block
from the code below with the `//----...` line.

## Language features

### Range algorithms

Prefer the C++20 range algorithms (`std::ranges::find`, `std::ranges::any_of`,
`std::ranges::sort`, ...) over `std::begin`/`std::end` combined with the `std::`
algorithms. The codebase is standardized on `std::ranges`; the range form is
shorter, supports projections, and cannot accidentally mix iterators from
different containers:

```
std::ranges::find(Container, Value);            // not std::find(std::begin(Container), std::end(Container), Value)
std::ranges::find(Items, Value, &Item::field);
```

### Enums

#### Enumerating values

When an enum has to be enumerated (to fill a combo box, to map the values to
config keys, to size an array, ...), add a `count` enumerator as the last item
and iterate `0 .. std::to_underlying(Type::count)`:

```
enum Mode
{
    first,
    second,
    third,

    count
};

for (const auto i: std::views::iota(0, std::to_underlying(Mode::count)))
    use(static_cast<Mode>(i));
```

`count` is needed only when the values are actually enumerated; other enums
don't have to define it.

#### switch over an enum

Handle every enumerator explicitly and terminate the switch with
`default: std::unreachable();`.

If only some values need special handling, let the rest share a behaviour
through `default:` (`default: break;` means "do nothing"), or merge `default`
with a fallback case:

```
switch (Value)
{
default:
case Type::unknown: return Fallback;
case Type::a:       return ...;
}
```

Use `[[fallthrough]]` when a case intentionally falls through into the next one.

## User interface

### Dialogs and localized labels

Localized text has a different width in each language, so measure labels at
runtime and derive the offsets from the text width. `HiStrlen` accounts for `&`
hotkeys and double-width characters (and returns `size_t`):

```
EditDlg[label].X1 = EditDlg[control].X2 + 2;
EditDlg[field].X1 = EditDlg[label].X1 + static_cast<int>(HiStrlen(msg(lng::Label))) + 1;
```

A control with a drop-down arrow (a combo box with a non-empty list, or an edit
with `DIF_HISTORY`) draws the arrow one column to the right of `X2`. When placing
an item immediately to the right, account for that column (`X2 + 3` keeps a
one-column gap); a control right-aligned to the dialog border needs no
adjustment, the arrow fits into the gap before the border.
