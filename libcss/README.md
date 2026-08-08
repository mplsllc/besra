LibCSS -- a CSS parser and selection engine
===========================================

Overview
--------

LibCSS is a CSS parser and selection engine. It aims to parse the forward
compatible CSS grammar.

Building
--------

LibCSS is vendored into Besra and built as part of the top-level CMake
build (see CLAUDE.md at the repo root); it is not built or installed
standalone. The generated selection source code (computed style data
accesses) is still generated at build time, now via a CMake custom command
rather than a `make select_generator` step.

API documentation
-----------------

Currently, there is none. However, the code is well commented and the
public API may be found in the "include" directory. The testcase sources
may also be of use in working out how to use it.

