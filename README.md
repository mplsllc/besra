<p align="center">
  <img src="art/besra-logo.svg" width="160" alt="Besra logo">
</p>

# Besra

**A lightweight, independent web engine for the readable modern web.**

Besra (*Accipiter virgatus*, a small, fast Asian sparrowhawk) is a browser built on
the NetSurf core, pushed well past where upstream NetSurf chooses to stop. It is
**not a fork in the "track upstream" sense**: it is a separate project that takes
NetSurf's genuinely independent rendering core as a starting point and evolves it
toward being a real contender in the engine field, with modern CSS, a real JS/DOM
layer, and standards coverage that upstream deliberately doesn't pursue.

Besra sits between stock NetSurf and Chrome/Firefox: heavier and more capable than
NetSurf, far lighter than a Chromium or Gecko browser. It's also the planned default
browser for the maintainer's new XFCE-based Linux distro.

## Thesis

The engine field has exactly one crowded lane (Blink/WebKit/Gecko, all effectively
Chromium-shaped, all backed by ad-adjacent corporations) and a lot of open space
beside it. There is real, demonstrated appetite for a **non-Chromium, non-corporate
engine** (Ladybird and the Servo revival prove the demand). Besra's bet:

> NetSurf already solved the *hard* part: a small, readable, genuinely independent C
> engine that parses and lays out real HTML/CSS on real sites. Most "new engine"
> projects spend years reaching that point. Besra starts there and spends its energy
> on what's *missing*, not on reinventing the foundation.

**What Besra is:** the best lightweight, independent, readable engine for the
readable modern web, fast, tiny, comprehensible, and actually keeping up with CSS
and (the differentiator) JavaScript/DOM.

**What Besra is not (yet):** a drop-in Chrome replacement for heavy JS web apps.
That gap is real and honest. The near-term target is *content sites done
excellently*, with the JS/DOM layer as the deliberate frontier that separates
Besra from stock NetSurf.

## Lineage

Besra inherits NetSurf's independent engine stack, **libcss** (CSS parse and
cascade), **libdom** (DOM), **libhubbub** (HTML5 parser), and the core
layout/redraw pipeline, none of it Chromium-derived. It also carries a large body
of engine work developed in **MacSurf** (the NetSurf port to Classic Mac OS 9):
modern CSS features upstream NetSurf never implemented.

## Why not upstream NetSurf?

Upstream's constraints are *philosophical, not technical*:

- **Portability over capability.** It targets RISC OS, AmigaOS, Atari, Haiku, so it
  rejects anything assuming a fast CPU, real RAM, or a modern toolchain. Besra is
  Linux-first (then Windows, then Mac) and free of that ceiling.
- **JavaScript is deliberately second class.** NetSurf ships JS *off by default*, on
  Duktape (ES5), with a thin DOM binding. That is a stance, not an oversight, and it
  is the single biggest gap between NetSurf and a usable modern browser. Besra
  treats JS/DOM as a first class problem.

Much of Besra's engine CSS work is genuinely upstreamable (the typography cluster in
particular). Where it makes sense we contribute upstream; where the ambition
diverges, we carry it here.

## Architecture

Besra is a **hard fork**, a root-sibling monorepo of the NetSurf engine components
(libcss, libdom, libhubbub, libparserutils, libwapcaplet, and support libraries)
plus the `netsurf` tree itself, built with **CMake**. The frontend is **Qt6**
(chosen as the cross-platform layer: the same C++ frontend targets Linux, Windows,
and macOS with native backends), currently being brought up alongside a transitional
GTK3 reference frontend that will be retired once Qt6 reaches parity.

Structurally, the biggest change from stock NetSurf is that the frontend
abstraction layer, NetSurf's `gui_table` function-pointer tables meant to support
many interchangeable frontends, has been collapsed into direct link-time calls,
since Besra ships exactly one frontend. See [CLAUDE.md](CLAUDE.md) for the details.

## Docs

- [docs/strategy.md](docs/strategy.md), the field, the positioning, the real failure
  mode (it's sustainability, not code), and how to avoid it.
- [docs/roadmap.md](docs/roadmap.md), leverage-ranked CSS/JS engine priorities. JS/DOM
  first.
- [docs/engine-basis.md](docs/engine-basis.md), what Besra inherits from NetSurf and
  MacSurf, what doesn't port, and the hard-fork decision.
- [docs/porting-from-macsurf.md](docs/porting-from-macsurf.md), how to build the
  cherry-pick manifest from the MacSurf engine commits.
- [plan.md](plan.md), the live, ground-truth build/architecture execution roadmap and
  current state.
- [CLAUDE.md](CLAUDE.md), architecture notes, build instructions, and project
  conventions.

## Status and roadmap

Past the founding/import stage. Currently building out the CMake-based engine core
and the Qt6 frontend in parallel; see `plan.md` for exactly where things stand.

Once the engine and Qt6 frontend are solid, the product roadmap adds what it takes
to be a real daily browser: built-in ad and tracker blocking, password management,
extensions, and cloud sync (leaning toward a trusted third party or
bring-your-own-storage rather than a self-hosted sync backend). Details in
`plan.md`.

## Building

```sh
mkdir build && cd build
cmake -DBESRA_FRONTEND=qt6 -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j$(nproc)
# binary: build/netsurf/frontends/qt6/netsurf-qt6
```

See [CLAUDE.md](CLAUDE.md) for build options, dependencies, and known gotchas.

## License

GPLv2, inherited from NetSurf.
