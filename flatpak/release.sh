#!/bin/bash
# Build the current source tree into the persistent flatpak repo served at
# https://flatpak.mp.ls/repo, so `flatpak update` picks it up on any machine
# with the "besra" remote added (see README.md in this directory for the
# remote-add/install commands).
#
# Unlike qtIRC's .deb release path (packaging/release.sh, which needs an
# aptly repo add + publish step), an ostree repo is just static files —
# flatpak-builder writes commits straight into it, and build-update-repo
# regenerates the summary/appstream metadata nginx serves from
# /home/patrick/.flatpak-repo. No separate publish step, no signing (v1:
# single maintainer, HTTPS already covers transport integrity).
#
# Usage: flatpak/release.sh

set -e
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="/home/patrick/.flatpak-repo/repo"

flatpak-builder --repo="$REPO" --force-clean "$ROOT/build-dir" \
    "$ROOT/com.mplsllc.Besra.yaml"

flatpak build-update-repo "$REPO"

echo "--- published to $REPO, served at https://flatpak.mp.ls/repo ---"
echo "verify with: curl -sI https://flatpak.mp.ls/repo/config"
