# Besra: Roadmap (leverage-ranked)

*Ordered by leverage: how much closer each item moves Besra to "usable modern engine,"
not by ease. The honest frame up front, then the ranked work. This is the CSS/JS
engine frontier specifically; for the build/architecture execution plan (CMake, the
Qt6 frontend, the direct-call refactor) see [plan.md](../plan.md) at the repo root.
For the product layer that sits on top once the engine is solid (ad blocking,
passwords, extensions, sync), see the roadmap notes in plan.md.*

## The honest frame

NetSurf's (and therefore Besra's starting) ceiling is **content sites, not web apps.**
"Lighter than Chrome" is real for reading/browsing: the engine is tiny and fast. The
thing that decides whether Besra is a *contender* or just *a nicer NetSurf* is the
**JS/DOM layer**. CSS is a finite, nearly-solved problem here; JS/DOM is the frontier
and the differentiator. Everything below is ranked with that in mind.

---

## 1. JS to DOM to layout round-tripping (the wall; do this first)

A real DOM binding where **JavaScript mutations reflow and repaint**. This is the
single thing everything modern depends on. Without it, JS runs but the page doesn't
respond to it, which is worse than no JS.

- MacSurf started this: QuickJS (ES2023) replaced Duktape, plus a reconvert-after-
  mutation path (JS-mutated content re-converts to boxes and repaints). That engine
  work, the QuickJS integration and the DOM-mutation-to-reflow bridge, is the seed.
- Besra's job: make it robust and complete on Linux, where you have real threads,
  real memory, and a real toolchain (none of the OS-9 cooperative-scheduler / temp-mem
  constraints that made it painful on Mac).
- Sub-parts: live DOM node bindings, event dispatch (click/submit/input/change) wired
  to real UI, `getComputedStyle` / `getBoundingClientRect` backed by real layout,
  mutation to invalidation to reflow.

## 2. `fetch` / XHR (no JS networking means no dynamic content)

JS-driven networking is table stakes for anything interactive. Today NetSurf's JS
can't make network requests. Wire `XMLHttpRequest` + `fetch()` to the core fetcher.
Depends on #1 being far enough along that responses can mutate the DOM meaningfully.

## 3. CSS maturity (mostly done, finish it)

Besra inherits a large CSS lead from MacSurf (see engine-basis). Remaining:

- **Intrinsic-sizing solver** (`min-content` / `max-content` / `fit-content`): the
  structural prerequisite that unblocks `table-layout: auto` and correct flex/grid
  shrink-to-fit. This is the highest-value *CSS* item; several others depend on it.
- Flex/grid edge cases: grid placement/span-aware sizing, `justify-items` /
  `align-items` content-sizing (the "stretch vs center" gap), `subgrid`.
- `appearance` + real form-control styling (the one big CSS-adjacent refactor:
  synthetic CSS-painted controls instead of native widgets).
- `background-clip: text`, `filter`, `clip-path`, transitions/animations: visual
  polish, degrade gracefully today.

On Linux you can drop the CW8/C89 constraints and the `cssh_css.c` preprocessor shims
that worked *around* libcss under CodeWarrior. Several of those can become native
libcss features (cleaner, upstreamable). Port the shims first (they work), refactor later.

## 4. HTML5 surface (incremental, high-frequency)

- Modern input types (`date`/`time`/`color`/`range`/`number`).
- `<picture>` / `srcset` responsive images.
- Canvas 2D (`<canvas>` + 2D context), needed by a surprising number of content sites.
- `<iframe>` (real, not stub).

## 5. Networking modernity

- HTTP/2 (many sites now assume it).
- Modern TLS story (on Linux you have real libraries, no need for the hand-rolled
  macTLS that OS 9 forced).
- Brotli/Zstd response decompression.

---

## Deliberately deferred / out of scope (be explicit)

- WebGL / WebRTC / Web Workers / Service Workers / WebSockets-as-core: heavy, and off
  the "readable web" thesis. Revisit only if the JS/DOM layer makes them cheap.
- Full web-app parity (Gmail-class SPAs). Honest non-goal near-term; state it plainly
  so the project isn't judged against a bar it isn't aiming at yet.

## Sequencing note

#1 (JS/DOM) and #3's intrinsic-sizing solver are the two "unblock everything" items.
Everything else is either downstream of them or incremental. Resist doing the easy
visible CSS polish *before* the sizing solver and the DOM bridge: that's building on
sand.
