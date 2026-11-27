#!/usr/bin/env bash
# build_module.sh <generated dir> <game> <sdk dir> <output .so|.dll> [C++ compiler command]
# Compiles the C++ written by mkrecomp into a loadable module. The SDK dir holds rc_abi.h,
# rc_module.h and 34010macros.h; any C++ compiler works (g++, clang++, "zig c++").
set -e
GEN=$1; GAME=$2; SDK=$(realpath "$3"); OUT=$(realpath -m "$4"); CXX=${5:-g++}
FLAGS="-std=c++17 -O2 -fvisibility=hidden -fno-strict-aliasing -w -I$SDK"
case "$OUT" in
  *.dll) LINK="-shared"; case "$CXX" in *zig*) FLAGS="$FLAGS -target x86_64-windows-gnu";; *) LINK="$LINK -static-libgcc -static-libstdc++";; esac ;;
  *) FLAGS="$FLAGS -fPIC"; LINK="-shared" ;;
esac
cd "$GEN"
JOBS=$(nproc 2>/dev/null || echo 4)
export FLAGS CXX
xargs -a "rc_$GAME.files" -P "$JOBS" -I{} sh -c '$CXX $FLAGS -c "{}" -o "{}.o" || exit 255'
$CXX $FLAGS $LINK -o "$OUT" $(sed 's/$/.o/' "rc_$GAME.files")
echo "$OUT"
