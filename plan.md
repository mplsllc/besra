# Besra Fork Roadmap

Cross-platform direction: Linux (XFCE distro default) first, then Windows, then Mac.
Qt6 is the portability layer, so platform-specific code should route through Qt/libcurl,
not bare glibc, so Win/Mac stay cheap later. See `docs/` for the founding
vision/strategy and roadmap, and `CLAUDE.md` for architecture and build notes.

## Completed
* **Step 1:** Cull all frontends except `gtk3` (kept as transitional reference only).
* **Step 2:** Collapse all 13 `gui_table` vtables into direct static calls. `guit`,
  `netsurf_table`, `netsurf_register()`, and `gui_factory.c`/`gui_table.h` removed.
* **Step 3:** Replaced the inherited `buildsystem` submodule and per-platform Makefiles
  with a unified CMake build. Explicit hardcoded source lists per library (no globbing)
  for reproducible builds.
* **Step 4:** Collapsed the `utils/` platform shims (dirent/regex/sys_time/inet
  compat headers) and the non-Linux fetch/scheduler abstractions down to their
  Linux-native forms.
* **Step 6 (started early, ahead of Step 5):** Qt6 frontend stood up against the
  direct-call core: plotters (QPainter), window (QWidget), bitmap (QImage), layout/
  font metrics (QFontMetrics + QTextLayout for exact shaping), scheduler (batching
  queue), fetcher fd polling (QSocketNotifier). `netsurf-qt6` builds, links, and
  **renders real HTML correctly** (headings, wrapped text, table layout), verified via
  `xvfb-run` + screenshot. `BESRA_FRONTEND` CMake option selects `gtk3` or `qt6`.

## Upcoming
* **Step 4b (deferred from Step 4):** The scheduler/fetch collapse so far targeted
  Linux-native glibc forms to unblock the build. Per the Qt-not-glibc rule, revisit so
  the *portable* layer (QTimer, libcurl sockets) is what Windows/Mac will actually use,
  don't let glibc-specific shortcuts calcify into the real platform layer.
* **Step 5:** Vendor the libraries (`libwapcaplet`, `libdom`, etc.) in as real flattened
  source, dropping the "independent projects" pretense (CMakeLists.txt per lib are already
  in place as of Step 3; this is the source-tree flattening itself).
* **Step 6 completion:** Finish Qt6 frontend feature-parity with what gtk3 had
  (toolbar/tabs/history/etc., currently core rendering + window plumbing only), then
  delete `gtk3` entirely and make `qt6` the sole frontend.
  * *Landmine (partially addressed):* `utils/nsoption.h` intertwined core and
    frontend-specific config macros (`nsgtk`); `options.h`/`nsoption.c/h` are being
    decoupled; confirm fully clean before deleting gtk3.

## Future / On Radar
* Replace `duktape` JS engine with `QuickJS` (target substitution in the new build system).
* Implement `libcss` and `libdom` regression tests behind the `BESRA_BUILD_TESTS` flag
  when CSS work begins.
* Product layer once the engine is solid: ad/tracker blocking (do first, cheap,
  on-thesis, `adblock.css` prior art), password management (mine MacSurf's
  `password-manager` branch), extensions (gated on JS/DOM maturity), cloud sync
  (lean: trusted third party / bring-your-own-storage E2E, not a rolled backend).

## Local build note
The local `build/` dir may carry a cached `-fsanitize=address` C flag from earlier
debugging (in `CMakeCache.txt`, not committed). If incremental builds fail on
`nsgenbind` (a build-time codegen tool) with AddressSanitizer leak reports, either
reconfigure without the sanitizer flag or run with `ASAN_OPTIONS=detect_leaks=0`.
