# Besra

**A lightweight, independent web engine for the readable modern web.**

Besra (*Accipiter virgatus* — a small, fast Asian sparrowhawk) is a browser engine
built on the NetSurf core, pushed hard past where upstream NetSurf chooses to stop.
It is **not a fork in the "track upstream" sense** — it is a separate project that
takes NetSurf's genuinely-independent rendering core as a starting point and evolves
it toward being a real contender in the engine field: modern CSS, a real JS/DOM
layer, and standards coverage that upstream deliberately doesn't pursue.

## Thesis

The engine field has exactly one crowded lane (Blink/WebKit/Gecko — all effectively
Chromium-shaped, all backed by ad-adjacent corporations) and a lot of open space
beside it. There is real, demonstrated appetite for a **non-Chromium, non-corporate
engine** (Ladybird and the Servo revival prove the demand). Besra's bet:

> NetSurf already solved the *hard* part — a small, readable, genuinely-independent
> C engine that parses and lays out real HTML/CSS on real sites. Most "new engine"
> projects spend years reaching that point. Besra starts there and spends its energy
> on what's *missing*, not on reinventing the foundation.

**What Besra is:** the best lightweight, independent, readable engine for the
readable modern web — fast, tiny, comprehensible, and actually keeping up with CSS
and (the differentiator) JavaScript/DOM.

**What Besra is not (yet):** a drop-in Chrome replacement for heavy JS web-apps.
That gap is real and honest — see [docs/roadmap.md](docs/roadmap.md). The near-term
target is *content sites done excellently*, with the JS/DOM layer as the deliberate
frontier that separates Besra from stock NetSurf.

## Lineage

Besra inherits NetSurf's independent engine stack — **libcss** (CSS parse + cascade),
**libdom** (DOM), **libhubbub** (HTML5 parser), and the core layout/redraw pipeline —
none of it Chromium-derived. It also carries a large body of engine work developed in
**MacSurf** (the NetSurf port to Classic Mac OS 9): modern CSS features upstream
NetSurf never implemented. See [docs/engine-basis.md](docs/engine-basis.md) for exactly
what carries over and what doesn't.

## Why not upstream NetSurf?

Upstream's constraints are *philosophical, not technical*:

- **Portability over capability.** It targets RISC OS, AmigaOS, Atari, Haiku — so it
  rejects anything assuming a fast CPU, real RAM, or a modern toolchain. Besra is
  Linux-first and free of that ceiling.
- **JavaScript is deliberately second-class.** NetSurf ships JS *off by default*, on
  Duktape (ES5), with a thin DOM binding. That is a stance, not an oversight — and it
  is the single biggest gap between NetSurf and a usable modern browser. Besra treats
  JS/DOM as a first-class problem.

Much of Besra's engine CSS work is genuinely upstreamable (the typography cluster in
particular — see engine-basis). Where it makes sense we contribute upstream; where the
ambition diverges, we carry it here.

## Docs

- [docs/strategy.md](docs/strategy.md) — the field, the positioning, the real failure
  mode (it's sustainability, not code), and how to avoid it.
- [docs/roadmap.md](docs/roadmap.md) — leverage-ranked priorities. JS/DOM first.
- [docs/engine-basis.md](docs/engine-basis.md) — what Besra inherits from NetSurf +
  MacSurf, what doesn't port, and the rebase-vs-hard-fork decision.
- [docs/porting-from-macsurf.md](docs/porting-from-macsurf.md) — how to build the
  cherry-pick manifest from the MacSurf engine commits.

## Status

Bootstrapping. This repo currently holds the vision and plan; the engine import is
the first work item (see engine-basis).
