# Besra: Engine Basis

*What Besra inherits, what carries over from MacSurf, what doesn't, and the mechanics
of staying (or not staying) close to upstream NetSurf. For current build status, see
[plan.md](../plan.md) and [CLAUDE.md](../CLAUDE.md) at the repo root.*

**Update since founding:** Besra committed to being a hard fork (no upstream remote).
The frontend decision below (GTK, framebuffer, or new) is resolved: **Qt6**, chosen
as the cross-platform layer for the Linux, then Windows, then Mac plan. A GTK3
frontend is kept temporarily as a transitional reference build and will be removed
once Qt6 reaches feature parity. The `gui_table` function-pointer indirection that
supported many interchangeable frontends has been collapsed into direct link-time
calls, since Besra ships exactly one frontend at a time. See CLAUDE.md for the
architecture detail.

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
platform-agnostic C and ports directly. Roughly ~70 commits/cycle live in
`libcss` + `content/handlers`, vs ~40 in the Mac-only frontend. Highlights that
upstream NetSurf does **not** have:

- **Typography cluster** (cleanest to port, and genuinely *upstreamable*):
  `text-align-last`, `hyphens` (soft-hyphen breaking), `text-justify` + real
  inter-word justification, `tab-size`. Upstream `layout_line()` literally leaves
  justify on the left.
- **CSS Logical Properties**: `margin/padding/border-block|inline`, `inset-*`,
  logical sizing.
- **CSS Custom Properties** (`var()`): native cascade-time resolution.
- **Grid**: auto-track content sizing, template parsing, alignment (V1).
- **background-clip** (border/padding/content-box), **background-size**,
  **image-rendering**, **text-decoration** longhands, **caret-color**,
  rgba-background compositing, box-alignment shorthands (`place-*`).
- **Inline-style rewrite parity**: modern CSS in `style=""` attributes gets the same
  treatment as stylesheet CSS.

### Two flavors of that work: port them differently

1. **Native libcss / core-layout features**: clean, port as-is, several are
   upstreamable. (Typography cluster, background-clip, image-rendering, logical props
   where done natively.)
2. **`cssh_css.c` preprocessor shims + `-macsurf-*` vendor properties**: MacSurf added
   a CSS-content-handler preprocessor that rewrites modern CSS into vendor properties
   (`-macsurf-grid`, `-macsurf-gradient`, `-macsurf-object-position`, animation-opacity,
   dotgrid/hstripe backgrounds, etc.) because doing it *natively* in libcss was
   expensive under CodeWarrior/C89 (bit-packing, dispatch tables, intern-crash class).
   These **work and port as-is**, but on Linux, with no CW8 and no C89, you can reimplement
   several as *native* libcss properties: cleaner, faster, upstreamable. **Port the
   shim first (it works), refactor to native later.** Don't block the import on the
   refactor.

The current `-macsurf-*` vendor properties (all backed by the cssh preprocessor):
`macsurf_grid`, `macsurf_grid_rows`, `macsurf_grid_col_span`, `macsurf_grid_flow`,
`macsurf_gradient`, `macsurf_object_position`, `macsurf_animation_opacity`,
`macsurf_animation_rotate`, `macsurf_hstripe_bg`, `macsurf_dotgrid`.

## What does NOT carry over

- The **entire `frontends/macos9` tree**: Carbon, QuickDraw, Open Transport, macTLS,
  the cooperative-scheduler and death-row memory machinery, the temp-mem GWorld dance.
  All of it exists to survive OS 9. Besra uses a Qt6 frontend instead (see the update
  note above).
- **QuickJS glue that's frontend-coupled**: the *engine-agnostic* JS/DOM bridge parts
  port; the Mac-specific integration does not. (See roadmap #1.)
- CW8/C89 workarounds: on Linux, delete them, use C99, real `snprintf`/`long long`,
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
attributed** (they already are, most carry `#NNN` issue tags and `fixesNNN` history)
so a rebase is a patch-replay, not a merge war.

## First work item (done)

The core (libhubbub/libdom/libcss/libparserutils/libwapcaplet + content/desktop) was
imported from a known-good NetSurf baseline, and a Linux frontend build stood up and
confirmed rendering. The `gui_table` vtable indirection was then fully collapsed into
direct calls, and the build moved from NetSurf's per-platform Makefiles to CMake. Next
up: replay the MacSurf engine features on top per
[porting-from-macsurf.md](porting-from-macsurf.md). See [plan.md](../plan.md) for the
live status of everything up to this point.
