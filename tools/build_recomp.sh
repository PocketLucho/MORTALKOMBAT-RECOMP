#!/usr/bin/env bash
# build_recomp.sh - statically recompile the games' TMS34010 programs from your own ROMs.
#
#   tools/build_recomp.sh <MKRecomp folder> [--windows] [mk mk2 mk3 umk3]
#
# Run it from a checkout of this repository at the same version as your MKRecomp build
# (Linux, SteamOS with a toolbox, or WSL on Windows). For each game whose ROMs are in
# <MKRecomp folder>/roms it:
#   1. builds the recompiler (tools/recomp/mkrecomp.cpp),
#   2. turns the program ROM into C++ (recursive descent from the CPU vectors + code
#      pointers found in the ROM + the addresses in recomp/<game>.miss, which the program
#      writes when it runs code the module did not cover),
#   3. compiles it into recomp/<game>.so (Linux) or, with --windows, recomp/<game>.dll
#      (needs x86_64-w64-mingw32-g++, e.g. `sudo apt install g++-mingw-w64-x86-64-posix`).
# The program loads the module when the game starts ("REC" in the menu); the result is
# identical to the interpreter, instruction for instruction.
#
# The generated C++ and the modules come from copyrighted ROMs: keep them for yourself,
# do not share or upload them.
set -e
ROOT=$(dirname "$(dirname "$(realpath "$0")")")
APP=$(realpath "${1:?usage: tools/build_recomp.sh <MKRecomp folder> [--windows] [games...]}")
shift
WIN=0
GAMES=()
for a in "$@"; do
  case "$a" in
    --windows) WIN=1 ;;
    *) GAMES+=("$a") ;;
  esac
done
[ ${#GAMES[@]} -eq 0 ] && GAMES=(mk mk2 mk3 umk3)
JOBS=$(nproc 2>/dev/null || echo 4)
command -v g++ >/dev/null || { echo "g++ is needed (sudo apt install build-essential)"; exit 1; }

cd "$ROOT"
echo "== building the recompiler"
make -s -j"$JOBS" build/linux/libcore.a build/linux/miniz.o
make -s build/linux/mkrecomp
if [ $WIN = 1 ]; then
  WCXX=${WINCXX:-x86_64-w64-mingw32-g++-posix}
  command -v "$WCXX" >/dev/null || WCXX=x86_64-w64-mingw32-g++
  command -v "$WCXX" >/dev/null || { echo "MinGW-w64 is needed (sudo apt install g++-mingw-w64-x86-64-posix)"; exit 1; }
  WAR=$(command -v x86_64-w64-mingw32-ar)
  make -s -j"$JOBS" BUILD=build/win-rc CXX="$WCXX" AR="$WAR" CXXFLAGS="-std=c++20 -O2 -Wno-deprecated-declarations" build/win-rc/libcore.a
  CORE=build/win-rc/libcore.a; EXT=dll; MCXX=$WCXX
else
  make -s -j"$JOBS" BUILD=build/pic CXXFLAGS="-std=c++20 -O2 -fPIC -Wno-deprecated-declarations" build/pic/libcore.a
  CORE=build/pic/libcore.a; EXT=so; MCXX=g++
fi

mkdir -p "$APP/recomp"
GEN=$(mktemp -d); trap 'rm -rf "$GEN"' EXIT
for g in "${GAMES[@]}"; do
  echo "== $g"
  mkdir -p "$GEN/$g"
  MISS=()
  [ -f "$APP/recomp/$g.miss" ] && MISS=("$APP/recomp/$g.miss")
  if ! build/linux/mkrecomp "$APP" "$g" "$GEN/$g" "${MISS[@]}"; then
    echo "   skipped (ROMs not found in $APP/roms)"
    continue
  fi
  tools/recomp/build_module.sh "$GEN/$g" "$g" "$CORE" "$APP/recomp/$g.$EXT" "$MCXX" >/dev/null
  echo "   -> $APP/recomp/$g.$EXT"
done
echo "Done. Play once, then run this again: the addresses the module missed (recomp/<game>.miss) are added."
