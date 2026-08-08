# Besra

A lightweight, independent web engine for the readable modern web — a hard fork of
NetSurf's engine (imported at 3.12-dev) plus the CSS/JS engine improvements developed in
MacSurf (the NetSurf port to Classic Mac OS 9). Positioned between stock NetSurf and
Chrome/Firefox: heavier and more capable than NetSurf (modern CSS, real JS/DOM), far
lighter than a Chromium/Gecko browser. See `README.md` and `.private/docs/` (gitignored)
for the full founding vision/strategy/roadmap.

**Cross-platform direction:** Linux first (default browser for the user's new
XFCE-based distro), then Windows, then Mac. **Qt6 is the portability layer** — this is
why the platform layer should route through Qt/libcurl, not bare glibc: Win/Mac should
become "recompile + package," not "write a new frontend."

## Repo layout

Root-sibling monorepo (hard fork, not submodules — each lib was imported as plain
source at a pinned upstream SHA and then diverges independently):

- `netsurf/` — the browser itself: `content/`, `desktop/`, `include/netsurf/`,
  `utils/`, `frontends/` (currently `gtk3` = transitional reference frontend, `qt6` =
  the real target, being brought to parity).
- `libcss/`, `libdom/`, `libhubbub/`, `libparserutils/`, `libwapcaplet/` — the engine
  core (CSS parse/cascade, DOM, HTML5 parse, string interning). The actual asset;
  being vendored in as real flattened source (in progress, see plan.md Step 5).
- `libnsgif/`, `libnsbmp/`, `libnsutils/`, `libnspsl/`, `libsvgtiny/`, `libutf8proc/`,
  `libnslog/` — supporting libs, also being vendored in.
- `libnsfb/` — orphaned by the frontend cull (was framebuffer-only); slated for removal.
- `nsgenbind/` — WebIDL binding generator (build-time host tool, generates the
  JS↔DOM glue consumed by `netsurf/content/handlers/javascript/duktape/`).
- `buildsystem/` — legacy NetSurf shared-Makefile system, superseded by the top-level
  CMake build; kept for reference during the transition.
- `plan.md` — the live, ground-truth execution roadmap. **Read this first** for
  current status — it's more current than this file for day-to-day state.
- `.private/` — gitignored. Founding docs (`docs/`), scratch/debug artifacts
  (`scratch/`), reference material pulled from elsewhere (`reference/`, e.g. the
  original NetSurf `qt` frontend kept for reading, not building against).

## Build

CMake, orchestrated from the repo root:

```sh
mkdir build && cd build
cmake -DBESRA_FRONTEND=qt6 -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j$(nproc)
# binary: build/netsurf/frontends/qt6/netsurf-qt6
```

- `-DBESRA_FRONTEND=gtk3|qt6` selects the frontend (default is currently `gtk3`, the
  transitional reference build — **qt6 is the active development target**, always pass
  it explicitly).
- `-DBESRA_BUILD_TESTS=ON` enables libcss/libdom check-based tests.
- Each vendored lib has its own `CMakeLists.txt` with an **explicit, hardcoded source
  list** (no globbing) for reproducible builds — when adding a source file to a
  vendored lib, add it to that lib's `CMakeLists.txt` (or the matching `*_srcs.txt`)
  by hand.
- Headless verify loop: `xvfb-run -a -s "-screen 0 1024x768x24" ./netsurf-qt6
  file:///path/to.html` — render output/paint events show up on stdout.

**Known build gotcha:** a stale `-fsanitize=address` C flag can end up cached in
`build/CMakeCache.txt` from earlier debugging and makes `nsgenbind` (a build-time
codegen tool) fail the build on harmless leak-detector reports (it's a short-lived
process; the leaks don't matter). If an incremental build fails at
`js-bindings`/`nsgenbind` with AddressSanitizer output, either delete `build/` and
reconfigure, or run with `ASAN_OPTIONS=detect_leaks=0`.

## Core architecture: the direct-call convention

The single biggest structural change from stock NetSurf: **the `gui_table` function-pointer
vtable indirection is gone.** Stock NetSurf routes every frontend operation through a
`struct netsurf_table` of function-pointer tables (`gui_window_table`, `gui_bitmap_table`,
etc.), assembled at startup via `netsurf_register()` into a global `guit`, so the core calls
`guit->window->redraw(...)`. That existed to support many interchangeable frontends
(RISC OS, Amiga, GTK, Windows...). Besra has exactly one frontend, so the indirection was
collapsed to **direct link-time calls**:

- Core code calls `gui_<area>_<op>(...)` directly (e.g. `gui_window_invalidate(...)`,
  `gui_bitmap_create(...)`, `gui_layout_width(...)`).
- The active frontend (`frontends/qt6/` or `frontends/gtk3/`) **defines** those symbols —
  no struct, no registration, no `guit`.
- Where the frontend doesn't implement an operation, the default implementation lives in
  `netsurf/desktop/gui_default.c` (not scattered `#ifdef`s).
- `desktop/gui_factory.c`, `desktop/gui_table.h`, `struct netsurf_table`, `guit`, and
  `netsurf_register()` are **deleted** — don't reintroduce them.

When adding a new frontend-provided operation: declare it as a plain function in the
relevant `include/netsurf/*.h` header, define it in the frontend, and add a default in
`gui_default.c` only if the frontend might not provide it.

## Conventions / hard rules

- **`content/handlers/html/layout.c` and friends (the layout engine) are semantically
  frozen** during the ongoing gut/refactor — no algorithmic changes. Mechanical
  call-site substitution (renaming, removing a parameter, direct-call conversion) is
  fine; changing layout behavior is not, until the refactor is complete and this note
  is revisited.
- **Platform-layer code should target Qt/libcurl, not bare glibc.** NetSurf's original
  platform shims supported 8 targets; when simplifying, don't replace that generality
  with Linux-only glibc calls — replace it with Qt's portable equivalents (QTimer,
  QSocketNotifier, QFile/QDir, QImage, QFontMetrics/QTextLayout) or libcurl, so
  Windows/Mac stay a recompile away rather than a rewrite. See plan.md's "Step 4b" note
  for the specific follow-up owed here.
- **Always commit** at each green (building) checkpoint — this is a standing
  instruction from the project owner, not something to ask about each time.
- Scratch/debug artifacts (one-off codegen scripts, build logs, screenshots, manual
  test HTML) go in `.private/scratch/` (gitignored), not the repo root.
- License: GPLv2 (inherited from NetSurf).

## Product roadmap (post-engine)

Once the engine/build foundation is solid: ad/tracker blocking (do first — cheap,
on-thesis, `frontends/gtk/res/adblock.css` is prior art), password management (mine
MacSurf's `password-manager` branch for prior art), extensions (gated on JS/DOM
maturity — Duktape→QuickJS swap first), cloud sync (lean: trusted third party /
bring-your-own-storage with client-side E2E, or Bitwarden-compatible for passwords
specifically — do not roll a sync backend; it makes the maintainer a custodian of
everyone's data, which is the wrong kind of ownership for a solo-maintained project).
