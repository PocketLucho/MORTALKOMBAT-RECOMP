#!/bin/bash
# MORTAL KOMBAT RECOMP - recompilador / recompiler (Linux / SteamOS)
# Recompila a C++ el programa TMS34010 de cada juego desde TUS ROMs (roms/) y lo compila en
# recomp/<juego>.so. Usa el compilador de C++ del sistema; si no hay (SteamOS), descarga
# Zig una sola vez (~45 MB, en tools/zig, sin root ni cambios en el sistema).
# Statically recompiles each game's TMS34010 program from YOUR ROMs into recomp/<game>.so.
# Uses the system C++ compiler, or downloads Zig once (SteamOS). Nothing is distributed.
#   ./build_recomp.sh [mk mk2 mk3 umk3]
set -e
cd "$(dirname "$(readlink -f "$0")")"
say() { printf '\n\033[31m== %s\033[0m\n   %s\n' "$1" "$2"; }
fail() { printf '\n\033[31mERROR: %s\n       %s\033[0m\n' "$1" "$2"; rm -rf tools/gen; [ -t 0 ] || sleep 8; exit 1; }
say "MORTAL KOMBAT RECOMP: recompilacion estatica" "MORTAL KOMBAT RECOMP: static recompilation"
MK=${MKRECOMP:-tools/recomp/mkrecomp}
SDK=${RC_SDK:-tools/recomp/sdk}
[ -x "$MK" ] || fail "Falta $MK." "$MK is missing."

ZIGVER=0.13.0
ZIGDIR="tools/zig/zig-linux-x86_64-$ZIGVER"
if [ -n "$CXX" ] && command -v ${CXX%% *} >/dev/null; then :
elif command -v c++ >/dev/null; then CXX=c++
elif command -v g++ >/dev/null; then CXX=g++
elif command -v clang++ >/dev/null; then CXX=clang++
else
  if [ ! -x "$ZIGDIR/zig" ]; then
    say "No hay compilador de C++: descargando Zig $ZIGVER (~45 MB, solo esta vez)..." "No C++ compiler: downloading Zig $ZIGVER (~45 MB, only this once)..."
    mkdir -p tools/zig
    URL="https://ziglang.org/download/$ZIGVER/zig-linux-x86_64-$ZIGVER.tar.xz"
    if command -v curl >/dev/null; then curl -fL --progress-bar -o tools/zig/zig.tar.xz "$URL" || fail "No se pudo descargar Zig (hay conexion?)." "Could not download Zig (are you online?)."
    elif command -v wget >/dev/null; then wget -q --show-progress -O tools/zig/zig.tar.xz "$URL" || fail "No se pudo descargar Zig (hay conexion?)." "Could not download Zig (are you online?)."
    else fail "No hay compilador ni curl/wget." "No compiler and no curl/wget."; fi
    tar -xJf tools/zig/zig.tar.xz -C tools/zig || fail "No se pudo descomprimir Zig." "Could not unpack Zig."
    rm -f tools/zig/zig.tar.xz
  fi
  CXX="$PWD/$ZIGDIR/zig c++"
fi
say "Compilador: $CXX" "Compiler: $CXX"

GAMES=("$@"); [ ${#GAMES[@]} -eq 0 ] && GAMES=(mk mk2 mk3 umk3)
JOBS=$(nproc 2>/dev/null || echo 4)
FLAGS="-std=c++17 -O2 -fPIC -fvisibility=hidden -fno-strict-aliasing -w -I$PWD/$SDK"
mkdir -p recomp
built=0
for g in "${GAMES[@]}"; do
  rm -rf tools/gen; mkdir -p tools/gen
  MISS=(); [ -f "recomp/$g.miss" ] && MISS=("recomp/$g.miss")
  if ! "$MK" . "$g" tools/gen "${MISS[@]}" 2>tools/gen/log.txt; then
    printf '   %s\n' "$(tail -1 tools/gen/log.txt)"; continue
  fi
  say "$g: compilando ($JOBS nucleos)..." "$g: compiling ($JOBS cores)..."
  export CXX FLAGS
  (cd tools/gen && xargs -a "rc_$g.files" -P "$JOBS" -I{} sh -c '$CXX $FLAGS -c "{}" -o "{}.o" || exit 255') \
    || fail "$g: error de compilacion." "$g: compilation error."
  (cd tools/gen && $CXX $FLAGS -shared -s -o "../../recomp/$g.so" $(sed 's/$/.o/' "rc_$g.files")) || fail "$g: error al enlazar." "$g: link error."
  printf '   -> recomp/%s.so\n' "$g"; built=$((built+1))
done
rm -rf tools/gen   # the generated C++ comes from the game: it is not kept
[ $built -gt 0 ] || fail "No se encontro ningun romset en roms/." "No romset found in roms/."
say "Listo. Los juegos recompilados salen con REC en el menu." "Done. Recompiled games show REC in the menu."
printf '   Juega un rato y vuelve a ejecutarlo: se anade el codigo que falto (recomp/*.miss).\n   Play for a while and run it again: the code that was missed (recomp/*.miss) is added.\n'
[ -t 0 ] || sleep 6
