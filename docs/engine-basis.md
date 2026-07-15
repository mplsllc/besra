# Besra — Engine Basis

*What Besra inherits, what carries over from MacSurf, what doesn't, and the mechanics
of staying (or not staying) close to upstream NetSurf.*

## The inherited core (from NetSurf, independent, non-Chromium)

Besra starts from NetSurf's engine stack. None of it is Chromium/WebKit-derived:

| Component | Role |
|---|---|
| **libhubbub** | HTML5 tokeniser + tree construction |
| **libdom** | DOM implementation |
| **libcss** | CSS parse + cascade + computed-style |
| **libparserutils / libwapcaplet** | parsing + interned-string primitives |
| **content/handlers/html** | box construction, layout, redraw (the layout engine) |
| **content/, desktop/** | content lifecycle, fetch orchestration, selection, etc. |

This is the "hard part" other independent engines spend years rebuilding. Besra gets
it working, on real content, on day one.

## What carries over from MacSurf (the engine work)

MacSurf is the NetSurf port to Classic Mac OS 9. Its **engine-side** work is
platform-agnostic C and ports directly — roughly ~70 commits/cycle live in
`libcss` + `content/handlers`, vs ~40 in the Mac-only frontend. Highlights that
upstream NetSurf does **not** have:

- **Typography cluster** (cleanest to port, and genuinely *upstreamable*):
  `text-align-last`, `hyphens` (soft-hyphen breaking), `text-justify` + real
  inter-word justification, `tab-size`. Upstream `layout_line()` literally leaves
  justify on the left.
- **CSS Logical Properties** — `margin/padding/border-block|inline`, `inset-*`,
  logical sizing.
- **CSS Custom Properties** (`var()`) — native cascade-time resolution.
- **Grid** — auto-track content sizing, template parsing, alignment (V1).
- **background-clip** (border/padding/content-box), **background-size**,
  **image-rendering**, **text-decoration** longhands, **caret-color**,
  rgba-background compositing, box-alignment shorthands (`place-*`).
- **Inline-style rewrite parity** — modern CSS in `style=""` attributes gets the same
  treatment as stylesheet CSS.

### Two flavors of that work — port them differently

1. **Native libcss / core-layout features** — clean, port as-is, several are
   upstreamable. (Typography cluster, background-clip, image-rendering, logical props
   where done natively.)
2. **`cssh_css.c` preprocessor shims + `-macsurf-*` vendor properties** — MacSurf added
   a CSS-content-handler preprocessor that rewrites modern CSS into vendor properties
   (`-macsurf-grid`, `-macsurf-gradient`, `-macsurf-object-position`, animation-opacity,
   dotgrid/hstripe backgrounds, etc.) because doing it *natively* in libcss was
   expensive under CodeWarrior/C89 (bit-packing, dispatch tables, intern-crash class).
   These **work and port as-is**, but on Linux — no CW8, no C89 — you can reimplement
   several as *native* libcss properties: cleaner, faster, upstreamable. **Port the
   shim first (it works), refactor to native later.** Don't block the import on the
   refactor.

The current `-macsurf-*` vendor properties (all backed by the cssh preprocessor):
`macsurf_grid`, `macsurf_grid_rows`, `macsurf_grid_col_span`, `macsurf_grid_flow`,
`macsurf_gradient`, `macsurf_object_position`, `macsurf_animation_opacity`,
`macsurf_animation_rotate`, `macsurf_hstripe_bg`, `macsurf_dotgrid`.

## What does NOT carry over

- The **entire `frontends/macos9` tree** — Carbon, QuickDraw, Open Transport, macTLS,
  the cooperative-scheduler + death-row memory machinery, the temp-mem GWorld dance.
  All of it exists to survive OS 9. Besra uses NetSurf's GTK or framebuffer frontend
  (mature, already better than the hand-rolled Mac frontend) or a new one.
- **QuickJS glue that's frontend-coupled** — the *engine-agnostic* JS/DOM bridge parts
  port; the Mac-specific integration does not. (See roadmap #1.)
- CW8/C89 workarounds — on Linux, delete them: use C99, real `snprintf`/`long long`,
  standard headers.

## The rebase-vs-hard-fork mechanics

Besra is a *separate project*, but you still choose how much upstream maintenance to
keep benefiting from:

- **Rebase-on-upstream (recommended while small):** track upstream NetSurf `master` in
  a remote; carry Besra's engine features as a patch series / feature branches; rebase
  periodically to inherit their security + bug fixes. Discipline cost, but you're not
  alone on TLS/parser/CVE maintenance.
- **Hard fork:** import once, never merge upstream again. Max freedom, max burden
  (you own every future security fix).

Either way, divergence compounds. Keep Besra's additions **modular and clearly
attributed** (they already are — most carry `#NNN` issue tags and `fixesNNN` history)
so a rebase is a patch-replay, not a merge war.

## First work item

Import the core (libhubbub/libdom/libcss/libparserutils/libwapcaplet + content/desktop)
from a known-good NetSurf baseline, stand up a Linux frontend build, confirm it renders,
then replay the MacSurf engine features on top per
[porting-from-macsurf.md](porting-from-macsurf.md).
