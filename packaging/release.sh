#!/bin/bash
# Build a Release .deb from the current source tree INSIDE a Debian
# container (so shlibdeps resolves to Debian package names like libqt6gui6,
# not this host's Ubuntu-only libqt6gui6t64 — MX Linux and other Debian-family
# distros can't install a .deb that depends on t64-suffixed packages), add it
# to the aptly repo, and republish so `apt update && apt upgrade` picks it up
# on apt.mp.ls. Bump the version in CMakeLists.txt's project() call first.
#
# Uses the prebuilt besra-builder image (packaging/Dockerfile.build) when
# available — the toolchain is baked in, so a release build skips the
# apt-get cost and goes straight to cmake/cpack. Falls back to bare
# debian:bookworm (build-in-debian.sh installs the toolchain itself in that
# case) if the image hasn't been built yet.
#
# To (re)build the fast image after changing the package list:
#   docker build -f packaging/Dockerfile.build -t besra-builder .
#
# Usage: packaging/release.sh

set -e
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="/tmp/besra_deb_out"

rm -rf "$OUT" && mkdir -p "$OUT"

BUILD_IMAGE="besra-builder"
DOCKERFILE="$ROOT/packaging/Dockerfile.build"

needs_build=0
if ! docker image inspect "$BUILD_IMAGE" >/dev/null 2>&1; then
    echo "besra-builder image not found — building it once." >&2
    needs_build=1
else
    image_epoch=$(date -d "$(docker image inspect -f '{{.Created}}' "$BUILD_IMAGE")" +%s)
    dockerfile_epoch=$(stat -c %Y "$DOCKERFILE")
    if [ "$dockerfile_epoch" -gt "$image_epoch" ]; then
        echo "Dockerfile.build is newer than the besra-builder image — rebuilding" \
             "so the toolchain matches." >&2
        needs_build=1
    fi
fi

if [ "$needs_build" = "1" ]; then
    docker build -f "$DOCKERFILE" -t "$BUILD_IMAGE" "$ROOT"
fi

docker run --rm \
  -v "$ROOT":/src:ro \
  -v "$ROOT/packaging/build-in-debian.sh":/build_in_debian.sh:ro \
  -v "$OUT":/out \
  -w /build \
  "$BUILD_IMAGE" bash -c "
mkdir -p /build
bash /build_in_debian.sh
cp /build/src/build/besra_*_amd64.deb /out/
"

DEB=$(ls "$OUT"/besra_*_amd64.deb | sort -V | tail -1)
sudo chown "$(id -u):$(id -g)" "$DEB"
echo "--- built: $DEB ---"
dpkg-deb -I "$DEB" | grep -E 'Version|Depends'

# aptly refuses to re-add a package at a version already in the repo (e.g.
# re-running a release without bumping CMakeLists.txt's VERSION) — drop the
# existing entry first so this is idempotent rather than a hard failure.
PKG_KEY="besra_$(dpkg-deb -f "$DEB" Version)_amd64"
aptly repo remove besra "$PKG_KEY" 2>/dev/null || true
aptly repo add besra "$DEB"

# Same idempotency issue at the published-pool level: aptly won't overwrite
# an already-linked file of the same name.
POOL_FILE="$HOME/.aptly/public/pool/main/b/besra/$(basename "$DEB")"
rm -f "$POOL_FILE"

aptly publish update besra

echo "--- published. verify with: ---"
echo "curl -s https://apt.mp.ls/dists/besra/main/binary-amd64/Packages"
