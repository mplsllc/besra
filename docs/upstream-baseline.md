# Besra — Upstream NetSurf Baseline

Hard-fork import from upstream NetSurf. Besra is a **separate repo** from here on;
this file only records *what commit each component was forked from* (for reference
and for anyone diffing Besra against stock NetSurf).

Imported: 2026-07-15 (NetSurf 3.12-dev baseline).
Source: git://git.netsurf-browser.org/<component>.git

| Component | Upstream commit at import |
|---|---|
| `buildsystem` | `771d7cb0691831aa5962855015b2f25ec527a9ee` |
| `libwapcaplet` | `c7c128d3eb3223b216c974471f82e9337fbcf4ba` |
| `libparserutils` | `6b0cbf086ca8eb8fe74b69f0c9ecf274eb2397ca` |
| `libhubbub` | `6651b8cf87a4aa87bcdb2ff024a02659cd3f9402` |
| `libcss` | `104d87fde48b9e022cd3cdad28aeb4d8cc0a0c5a` |
| `libdom` | `f69781e1f062444b5af3f62d431d7d94018da53b` |
| `nsgenbind` | `44c6736937ae17d4065d02959b82813b8f06a51e` |
| `libnsutils` | `0bd39060740b6163bd50875326654a722df97eb2` |
| `libnslog` | `9c19d16e196a5226ed89f3aa8c4befffeff716fe` |
| `libnsgif` | `24ddb6421c31669bc10f73308825ac9dd4e29229` |
| `libnsbmp` | `ea063c9f46acb43e90208da14073332b505ef7e7` |
| `libnspsl` | `b170f84028fb01364f89020f53e4cc30c10f2fd2` |
| `libutf8proc` | `0d22740b0de2636307cb95c928b244263ed17caa` |
| `libsvgtiny` | `7ede71b572a1672ee9ddb7b3e626ddb17b4c8170` |
| `libnsfb` | `b701cdce7241c3747ccd78658a365db0983ebe24` |
| `netsurf` | `a471a0d44274ec57fee5e5f30ae59fbd2ad02656` |

## Post-import additions

Components vendored after the initial NetSurf import, same hard-fork convention
(real source, pinned SHA recorded here, no upstream remote kept):

| Component | Upstream commit at import | Source |
|---|---|---|
| `libquickjs` | `954dc53628e36891f93c359aa60895c2ae3dac6b` | `https://github.com/quickjs-ng/quickjs.git` (imported 2026-08-08) |

`libnsfb` (row above) was later deleted; see plan.md Step 6.
