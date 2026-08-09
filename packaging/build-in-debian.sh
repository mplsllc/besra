#!/bin/bash
# Runs INSIDE the besra-builder container (packaging/Dockerfile.build),
# invoked by release.sh. Not meant to be run directly on the host.
#
# The toolchain (cmake, flex, bison, qt6-base-dev, etc.) is already baked
# into the besra-builder image — see packaging/Dockerfile.build — so this
# script only does the actual build. If run against a bare debian:bookworm
# image (no besra-builder built yet), it installs the toolchain itself as
# a fallback so it still works, just slower.
set -e

if ! command -v cmake >/dev/null 2>&1; then
    apt-get update -qq
    apt-get install -y -qq cmake build-essential flex bison pkg-config dpkg-dev file \
      qt6-base-dev \
      libcurl4-openssl-dev libssl-dev libxml2-dev libpng-dev libjpeg62-turbo-dev zlib1g-dev
fi

cp -r /src /build/src
cd /build/src

rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"

cd build
cpack -G DEB
