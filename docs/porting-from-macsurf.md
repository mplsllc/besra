# Besra: Porting from MacSurf

*How to turn MacSurf's engine work into a clean import for Besra. The goal is a
**port manifest**: every relevant change grouped by how it lands on a stock NetSurf
base, so day one is `git cherry-pick`, not archaeology.*

**Status:** the core baseline import and Linux frontend bring-up (step 1 below) are
done; see [plan.md](../plan.md) for current state. This manifest work (steps 2 to 4)
has not started yet.

MacSurf repo: `/home/patrick/Webs/macsurf` (branch `css-coverage` had the latest CSS
work as of the founding conversation). Its engine commits are tagged with GitHub issue
numbers (`#NNN`) and a `fixesNNN` running history.

## The three buckets

Every MacSurf change falls into one of three buckets for Besra:

### A. Clean cherry-pick (platform-agnostic engine code)
Lives in `browser/libcss/**` or `browser/netsurf/content/**` and doesn't depend on the
Mac frontend or a `-macsurf-*` vendor property. These replay onto upstream NetSurf
with little/no conflict.

Examples: the typography cluster (`text-align-last`, `hyphens`, `text-justify`,
`tab-size`), `background-clip`, `image-rendering`, `text-decoration` longhands,
logical-property handling, rgba-background compositing.

### B. Needs the cssh preprocessor / vendor property first
Lives in `content/handlers/css/cssh_css.c` or reads a `css_computed_macsurf_*`
accessor. Works, but depends on MacSurf's preprocessor + vendor-property scaffolding.
**Port the scaffolding once, then these come along**, or reimplement the feature as a
*native* libcss property on Linux (no CW8 constraint) and drop the shim.

Examples: grid (`-macsurf-grid*`), gradients (`-macsurf-gradient`), object-position,
animation-opacity/rotate, the hstripe/dotgrid background textures.

### C. Mac-only, do not port
`browser/netsurf/frontends/macos9/**`, macTLS, Open Transport, QuickDraw plotters, the
death-row/op_depth memory machinery, CW8/C89 workarounds. Besra replaces all of this
with a Linux frontend + modern libraries.

## Generating the manifest (mechanical)

From the MacSurf repo, classify the engine commits since the last shared baseline:

```sh
cd /home/patrick/Webs/macsurf

# Bucket A candidates: touch ONLY libcss or content/, never the mac frontend
git log --oneline <BASELINE>..css-coverage --name-only \
  | ...   # filter commits whose files are all under browser/libcss/ or
          #        browser/netsurf/content/ and none under frontends/macos9/

# Bucket B: commits that touch content/handlers/css/cssh_css.c OR reference
#           css_computed_macsurf_ / -macsurf- ; these need the shim scaffolding
git log --oneline <BASELINE>..css-coverage -- browser/netsurf/content/handlers/css/cssh_css.c

# Bucket C: anything under frontends/macos9/, skip
git log --oneline <BASELINE>..css-coverage -- browser/netsurf/frontends/macos9/
```

`<BASELINE>` = the upstream NetSurf commit MacSurf originally branched from (find via
`git log` for the initial import, or diff against the closest upstream tag).

Practical approach when Besra exists: don't literally cherry-pick 100+ commits. Instead
**diff the final MacSurf engine tree against the upstream baseline per subsystem**
(libcss typography, layout grid, redraw background, etc.) and apply as a handful of
squashed feature patches. Cleaner history, easier to rebase later.

## Order of import

1. **Core baseline (done)**: import upstream NetSurf's libhubbub/libdom/libcss/
   libparserutils/libwapcaplet + content/desktop at a known-good tag. Stand up a
   Linux frontend (Qt6, with a transitional GTK3 reference build) and confirm it
   renders a real page. *No MacSurf code yet, establish a clean, building base.*
2. **Bucket A features**: replay the platform-agnostic CSS wins. Each is independently
   testable against a reference render (see below). Highest value / lowest risk.
3. **Bucket B scaffolding decision**: for each vendor-property feature, decide
   *port-the-shim* vs *reimplement-native*. On Linux, native is often the better call
   now that CW8/C89 is gone. Grid is the biggest one.
4. **JS/DOM**: the roadmap #1 frontier. Bring over the engine-agnostic QuickJS + DOM-
   mutation→reflow bridge; complete it on Linux. This is where Besra earns its thesis.

## Verification harness (carry this habit over)

MacSurf's most effective QA loop was: build a focused `t.html` exercising one feature,
render it in **headless Chrome** as the reference, compare. On Linux this is even
easier, you can render Besra headless too and diff pixel-for-pixel. Keep that loop;
it's how the CSS work stayed honest.

Headless Chrome reference (already available on this machine):
`~/.cache/ms-playwright/chromium_headless_shell-1208/chrome-headless-shell-linux64/chrome-headless-shell --headless --screenshot=out.png <url>`
