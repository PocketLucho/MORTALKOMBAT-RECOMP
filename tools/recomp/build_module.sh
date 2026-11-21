#!/usr/bin/env bash
# build_module.sh <generated dir> <game> <libcore.a built with -fPIC (or for Windows)> <output> [compiler]
# Compiles the C++ written by mkrecomp into a loadable module (.so on Linux, .dll with MinGW).
set -e
GEN=$1; GAME=$2; CORE=$(realpath "$3"); OUT=$(realpath -m "$4"); CXX=${5:-g++}
ROOT=$(dirname "$(dirname "$(dirname "$(realpath "$0")")")")
FLAGS="-std=c++20 -O2 -fvisibility=hidden -Wno-deprecated-declarations -I$ROOT/src/emu -I$ROOT/src/mame -I$ROOT/src"
case "$OUT" in *.dll) LINK="-shared -static -static-libgcc -static-libstdc++";; *) FLAGS="$FLAGS -fPIC"; LINK="-shared";; esac
cd "$GEN"
JOBS=$(nproc 2>/dev/null || echo 4)
xargs -a "rc_$GAME.files" -P "$JOBS" -I{} sh -c "$CXX $FLAGS -c {} -o {}.o || exit 255"
$CXX $LINK -o "$OUT" $(sed 's/$/.o/' "rc_$GAME.files") "$CORE"
echo "$OUT"
