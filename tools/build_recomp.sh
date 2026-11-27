#!/usr/bin/env bash
# build_recomp.sh - developer version of the release's build_recomp.sh / RECOMPILAR_RECOMPILE.bat:
# statically recompiles the games' TMS34010 programs from your own ROMs, from a checkout.
#
#   tools/build_recomp.sh <MKRecomp folder> [--windows] [mk mk2 mk3 umk3]
#
# Builds tools/recomp/mkrecomp, generates C++ from <folder>/roms (plus recomp/<game>.miss,
# the addresses the program logged) and compiles recomp/<game>.so, or with --windows
# recomp/<game>.dll (x86_64-w64-mingw32-g++, or CXX="zig c++"). The release packages
# already include a prebuilt mkrecomp and a script that needs no compiler installed.
#
# The generated C++ and the modules come from copyrighted ROMs: keep them for yourself.
set -e
ROOT=$(dirname "$(dirname "$(realpath "$0")")")
APP=$(realpath "${1:?usage: tools/build_recomp.sh <MKRecomp folder> [--windows] [games...]}")
shift
WIN=0; GAMES=()
for a in "$@"; do case "$a" in --windows) WIN=1 ;; *) GAMES+=("$a") ;; esac; done
[ ${#GAMES[@]} -eq 0 ] && GAMES=(mk mk2 mk3 umk3)
JOBS=$(nproc 2>/dev/null || echo 4)
cd "$ROOT"
echo "== building the recompiler"
make -s -j"$JOBS" build/linux/libcore.a build/linux/miniz.o
make -s build/linux/mkrecomp
SDK=build/rc_sdk; mkdir -p $SDK
cp src/recomp/rc_abi.h src/recomp/rc_module.h src/mame/cpu/tms34010/34010macros.h $SDK/
if [ $WIN = 1 ]; then
  EXT=dll; CXX=${CXX:-x86_64-w64-mingw32-g++-posix}
  command -v ${CXX%% *} >/dev/null || CXX=x86_64-w64-mingw32-g++
else
  EXT=so; CXX=${CXX:-g++}
fi
mkdir -p "$APP/recomp"
GEN=$(mktemp -d); trap 'rm -rf "$GEN"' EXIT
for g in "${GAMES[@]}"; do
  echo "== $g"
  rm -rf "$GEN/$g"; mkdir -p "$GEN/$g"
  MISS=(); [ -f "$APP/recomp/$g.miss" ] && MISS=("$APP/recomp/$g.miss")
  build/linux/mkrecomp "$APP" "$g" "$GEN/$g" "${MISS[@]}" || { echo "   skipped"; continue; }
  tools/recomp/build_module.sh "$GEN/$g" "$g" $SDK "$APP/recomp/$g.$EXT" "$CXX" >/dev/null
  echo "   -> $APP/recomp/$g.$EXT"
done
