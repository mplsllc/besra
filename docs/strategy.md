# Besra — Strategy

*Captured from the founding conversation. This is the "why" and the "how not to die,"
not the "what to build" (that's [roadmap.md](roadmap.md)).*

## The field, honestly

The browser-engine field looks like a monoculture: Blink, WebKit, Gecko — three
engines, and in practice everything ships Chromium. That monoculture is exactly the
opening. There is real, demonstrated appetite for an engine **not owned by an ad
company**:

- **Ladybird** (LibWeb) is pulling serious attention and funding on precisely that
  pitch — an independent, from-scratch engine.
- **Servo** got revived after Mozilla dropped it.

The lesson isn't "copy them." It's that the independent-engine niche is *not dead* —
it's a field with one crowded lane and open space beside it, and there's momentum in
the space right now. That's wind at the back.

## NetSurf's underappreciated asset

The hard part of an engine is the part that doesn't demo: parse → cascade → layout →
paint, on real messy HTML/CSS. Ladybird spent *years* getting a from-scratch engine to
render real pages. **NetSurf has had that working, on real content, for a long time**,
in a small readable C codebase with its own independent libcss/libdom/hubbub.

So Besra does not start at zero. It starts at *"solid, genuinely-independent engine
that stopped evolving."* That is a dramatically better starting line than a blank repo —
you inherit the three years of unglamorous foundation work for free.

The stagnation Besra reacts to is **not a code problem — it's a velocity and narrative
problem.** Stock NetSurf moves slowly and conservatively. The engine is good; the
forward motion isn't there.

## What "contender" actually means

Not "beats Blink on Gmail." The realistic, defensible bar:

1. **Unambiguously the best at a defensible thing** — the lightweight, independent,
   *readable* engine that actually handles the modern *readable* web. Fast, tiny,
   comprehensible.
2. **Visible momentum.** An engine that ships modern CSS + real JS/DOM on a regular
   cadence tells a story stock NetSurf can't — and momentum is what attracts the one
   thing NetSurf lacks: contributors.

Positioned as "fast lightweight browser for the readable web," Besra's work makes it
markedly better than stock NetSurf. Positioned as "full Chrome replacement," the
JS/DOM gap is a real wall (see roadmap). Pick the honest framing; it's still a genuine
niche nobody occupies — Chrome/Firefox are heavy, and the other lightweight engines
(Dillo, etc.) don't do modern JS at all.

## The real failure mode (read this twice)

**The binding constraint is not the engine. It's the maintainer.**

Solo-maintaining a browser engine is how these projects die — not by hitting a
technical wall, but by one person carrying the entire security + standards + compat
surface until the steam runs out. The projects that break through convert *early
momentum into other people's hands* — a contributor or two, some funding, a community —
**before** the founder burns out.

Therefore the most strategic early investment is **not a feature — it's making Besra
legible and joinable:**

- Clear architecture docs (what the pieces are, how a page flows through them).
- A sharp "here's the thesis and the roadmap" narrative (this repo is the start).
- Low-friction build (one command, documented deps).
- Visible, dated wins (a changelog / release notes cadence).

The instinct is already there — MacSurf shipped a CSS support tracker and real release
notes. Point that same energy at *"why contribute here"* and a good project becomes a
contender. **Build it to be joined.**

## The ownership decision (decide early)

A separate project means Besra owns its security surface: TLS, parser fuzzing, the JS
engine's CVEs, the lot. You stop getting upstream NetSurf's fixes for free. Two stances:

- **Rebase periodically on upstream** — keep their bugfixes, carry Besra's features as
  a patch series on top. Less lonely, more discipline required. *Recommended while
  small.*
- **True hard fork** — full ownership, full divergence, no upstream merges. Maximum
  freedom, maximum burden.

Divergence compounds either way; the question is how much of upstream's ongoing
maintenance you want to keep benefiting from. See engine-basis for the mechanics.
