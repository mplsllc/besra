# Besra Fork Roadmap

Cross-platform direction: Linux (XFCE distro default) first, then Windows, then Mac.
Qt6 is the portability layer, so platform-specific code should route through Qt/libcurl,
not bare glibc, so Win/Mac stay cheap later. See `docs/` for the founding
vision/strategy and roadmap, and `CLAUDE.md` for architecture and build notes.

## Completed

* **Step 1:** Cull all frontends except `gtk3` (kept as transitional reference only;
  since removed, see Step 6 below).
* **Step 2:** Collapse all 13 `gui_table` vtables into direct static calls. `guit`,
  `netsurf_table`, `netsurf_register()`, and `gui_factory.c`/`gui_table.h` removed.
* **Step 3:** Replaced the inherited `buildsystem` submodule and per-platform Makefiles
  with a unified CMake build. Explicit hardcoded source lists per library (no globbing)
  for reproducible builds.
* **Step 4:** Collapsed the `utils/` platform shims (dirent/regex/sys_time/inet
  compat headers) and the non-Linux fetch/scheduler abstractions down to their
  Linux-native forms.
* **Step 4b:** The Step 4 shim collapse had gone further than intended (deleted the
  `gui_fetch_socket_open/close` frontend hooks entirely and inlined raw POSIX
  `socket()`/`close()` into `curl.c`; `close()` on a Windows `SOCKET` is actually wrong).
  Added `utils/inet.h` as the one place raw platform socket headers are included from
  (winsock2.h on Windows, POSIX headers elsewhere) with `ns_close_socket` as the
  portable close primitive. Also fixed two real bugs in the qt6 frontend's fetch
  integration: the `QSocketNotifier` callbacks were empty no-ops, and `main.cpp` never
  called `QApplication::exec()` (a hand-rolled polling loop instead). Replaced with a
  `FetchPump` QObject: functional notifier callbacks, a heartbeat `QTimer`, real
  `app.exec()`.
* **Step 6: Qt6 full UI parity, gtk3 deleted.** The Qt6 frontend is now a complete,
  functional browser, not a rendering-only stub:
  - Real embedded resources (`res/besra.qrc`: default.css/internal.css/icons/message
    catalogue), replacing a hardcoded developer-machine path.
  - `BesraWindow` (QMainWindow): menu bar, navigation toolbar (back/forward/
    reload-stop/URL bar), tab strip, status bar. `gui_window_create` routes through
    `BesraWindow::createTabOrWindow` per the `GW_CREATE_TAB`/`FOREGROUND`/
    `FOCUS_LOCATION` flags.
  - `BrowserTab`/`NSWidget`: real scrolling (QScrollArea + actual scrollbars), real
    mouse/keyboard/wheel input forwarded into `browser_window_mouse_click/track`/
    `browser_window_key_press`.
  - `CoreWindowWidget`: a generic implementation of the `core_window` contract
    (history/bookmarks/cookies/page-info all render through this, the core's own
    treeview code, drawn via the same plotter table as page rendering). Unlocked
    History, Bookmarks, and Cookies together.
  - Preferences (nsoption-backed, real read/write round-trip), Download manager
    (`gui_download_*`, QFileDialog save-as + progress list), Print (QPrintDialog +
    the existing plotter table), Find-in-page, View Source, About, clipboard
    (QClipboard), external-URL launch (QDesktopServices).
  - Verified throughout via `xvfb-run` + `xdotool` (real menu clicks and dialog
    opens, not just clean compiles) + screenshot capture.
  - gtk3 and the orphaned `libnsfb` deleted; confirmed `desktop/options.h` and
    `utils/nsoption.c/.h` carry no gtk-specific references first (the landmine
    flagged since Step 4). `netsurf/CMakeLists.txt` no longer has a frontend choice.
  - Build target renamed `netsurf-qt6` → `besra`.

* **Step 5:** Vendored libraries (`libwapcaplet`, `libparserutils`, `libhubbub`,
  `libdom`, `libcss`, `libnsgif`, `libnsbmp`, `libnsutils`, `libnspsl`, `libsvgtiny`)
  no longer carry the "independent project" pretense. Earlier in Step 5, each lib's
  dead standalone build scaffolding (`Makefile`, `Makefile.config`, `libFOO.pc.in`,
  superseded entirely by the per-lib `CMakeLists.txt` from Step 3) was removed. This
  pass finished the job: 7 of the 10 READMEs (libwapcaplet, libparserutils, libhubbub,
  libdom, libcss, libsvgtiny, libnsutils) still described a standalone GNU-make build,
  down to `svn co`-ing sibling libraries and installing to `/usr/local`, actively
  wrong now, not just stale (e.g. libhubbub's README told you to fetch and build your
  own separate libparserutils). Replaced those sections with a short, accurate note
  pointing at the top-level CMake build; kept the genuinely-still-useful content
  (overview, rationale, API usage, test-driver pointers). Also cleared out stray local
  `build/` artifact directories (already gitignored, never tracked, just disk clutter).
  No source-tree/`#include`-path merge was done: the per-lib `include/<name>/` +
  `src/` layout stays as-is; that's a large, high-risk, low-value mechanical rewrite
  with no concrete benefit distinct from what's already fixed here.

* **Step 6 follow-ups, closed out:** the four gaps flagged when Step 6 landed are now
  real, not just fixed-in-name:
  - Bookmarks now persist to `<AppConfigLocation>/besra/hotlist` (via
    `QStandardPaths::AppConfigLocation`), saved on `QApplication::aboutToQuit`.
    Verified end-to-end: added a bookmark, quit, relaunched, the entry (and the
    "Unsorted entries" folder it lives in) survived. Along the way, found that
    `hotlist_add_url()` needs `hotlist_manager_init()` (a corewindow attached), not
    just `hotlist_init()`, to succeed at all, and that it correctly declines internal
    pseudo-pages like `resource:welcome.html` (no bookmarking `about:blank`, same as
    any real browser) -- neither is a bug, both now documented in panels.cpp.
  - `gui_window_set_icon` builds a `QIcon` from the content's bitmap
    (`content_get_bitmap` + bitmap.cpp's `gui_bitmap_get_qimage`) and reflects it in
    the tab. Verified: no crash on a real `<link rel="icon">` page, icon visible in
    the tab strip.
  - `gui_window_create_form_select_menu` is a real `QMenu` popup now: one checkable
    action per `form_option`, positioned at the control's bounding rect, committing
    via `form_select_process_selection` on click. Verified: a real `<select>` renders
    and its popup shows all options with the correct one checked.
  - Find-in-page's forward/back buttons now reflect `gui_search_forward/back_state`
    via a small per-dialog registry keyed on the same context pointer
    `browser_window_search()` is given (dialogs.cpp).

## Upcoming

(nothing currently queued; see Future / On Radar for the next tier of work)

## Future / On Radar

* Replace `duktape` JS engine with `QuickJS` (target substitution in the new build system).
* Implement `libcss` and `libdom` regression tests behind the `BESRA_BUILD_TESTS` flag
  when CSS work begins.
* Product layer once the engine is solid: ad/tracker blocking (do first, cheap,
  on-thesis; the old `adblock.css` resource is already embedded as prior art),
  password management (mine MacSurf's `password-manager` branch), extensions (gated
  on JS/DOM maturity), cloud sync (lean: trusted third party / bring-your-own-storage
  E2E, not a rolled backend).

## Local build note

The local `build/` dir may carry a cached `-fsanitize=address` C flag from earlier
debugging (in `CMakeCache.txt`, not committed). If incremental builds fail on
`nsgenbind` (a build-time codegen tool) with AddressSanitizer leak reports, either
reconfigure without the sanitizer flag or run with `ASAN_OPTIONS=detect_leaks=0`.
