# Mortal Kombat Recomp

*Castellano: [README.es.md](README.es.md)*

Midway's **Mortal Kombat** arcade saga (MK, MK II, MK3 and UMK3) playable in a single
Windows / Linux application with a game menu, CRT shaders, widescreen modes and online play.
It is built on the same base as [NBA JAM Recomp](https://github.com/PocketLucho/NBA-JAM---Recompiled),
which already reproduces Midway's two arcade boards (T-Unit and Wolf Unit).

Created by **Pocket_lucho** with **Claude** (Anthropic).
If you want to support me, go to https://buymeacoffee.com/pocket_lucho

| Game | Board | MAME romset | Status |
|---|---|---|---|
| Mortal Kombat (1992), rev 5.0 / 4.0 T-Unit | T-Unit + Williams ADPCM sound (6809, YM2151, OKI6295) | `mk.zip` | playable, with sound |
| Mortal Kombat II (1993), rev L3.1 | T-Unit + DCS 2K sound (ADSP-2105) | `mk2.zip` | playable, with sound |
| Mortal Kombat 3 (1995), rev 1.0 / 2.0 / 2.1 | Wolf Unit + DCS 8K | `mk3r10.zip` (or `mk3.zip`) | playable, with sound |
| Ultimate Mortal Kombat 3 (1995), rev 1.2 / 1.1 / 1.0 | Wolf Unit + DCS 8K | `umk3.zip` | playable, with sound |

All four were tested with the MAME romsets: they boot, pass the protection checks, run the
attract mode and reach the fights with picture and sound. All four can be **statically
recompiled** from your ROMs (see below).

## Static recompilation (from your ROMs)

Mortal Kombat's arcade source code was never released, so there is no "built from source"
here as with some NBA JAM games. Instead, `tools/build_recomp.sh` **statically recompiles**
each game's TMS34010 program from your own ROMs:

1. `tools/recomp/mkrecomp` reads the program from `roms/`, finds the code (from the CPU's 32
   vectors, branch and call targets, code pointers stored in the ROM and the addresses the
   program logs in `recomp/<game>.miss`) and writes C++: every instruction is a label;
   branches (JRcc, JAcc, DSJ, DSJS) are native conditions and `goto`s; register instructions
   use the interpreter's own macros with the opcode as a constant; the rest call their
   handler directly, already decoded.
2. That C++ is compiled into a module: `recomp/<game>.dll` (Windows) or `.so` (Linux / SteamOS).
   The module only uses a C interface (pointers to the CPU state), so any compiler works: the
   system's, or Zig, which the script downloads only when there is none.
3. When the game starts the application loads the module (after checking it was built from
   those ROMs and the same program version) and the menu shows **REC**. Anything not
   recompiled (code in RAM, addresses not found) runs in the interpreter and is logged in
   `recomp/<game>.miss` for the next build.

After every instruction the cycle budget and the PC are checked as in the interpreter loop,
so the result is **identical instruction for instruction**: in the tests (1,800-2,400 frames
of attract mode and fights) picture and RAM match the interpreter on every frame, and after
one feedback pass 100% of the executed code runs from the module (MK: ~27,700 recompiled
instructions, MK II ~34,400, MK3 ~43,800, UMK3 ~49,100).

**The tool comes in the release**, ready to use, nothing to install:

* **Windows**: double-click `RECOMPILAR_RECOMPILE.bat` (the first time it downloads Zig, ~75 MB).
* **Linux / SteamOS**: double-click `Recompilar - Recompile.desktop` or run `./build_recomp.sh`
  (uses the system compiler; on SteamOS it downloads Zig once, ~45 MB, into `tools/zig`).

It builds `recomp/<game>.dll|.so` for every game whose ROMs are in `roms/` and deletes the
generated C++ afterwards. Play for a while and run it again to include what was missed. **The
modules come from copyrighted ROMs: do not share or upload them.**

From the source code: `tools/build_recomp.sh <MKRecomp folder> [--windows]`.

## No game data included

Midway's code, graphics, sound and music are copyrighted, so neither this repository nor the
builds contain them. Every user copies their own MAME romsets into `roms/`: `mk.zip`,
`mk2.zip`, `mk3r10.zip` (or `mk3.zip`), `umk3.zip`.

The application scans `roms/` (sub-folders and ZIPs included), identifies each file by its
CRC32 (the file name does not matter) and assembles what each game needs when it starts. If
something is missing the game is greyed out in the menu and selecting it says what to copy.

## Menu

In the style of [Planet Harriers](https://github.com/PocketLucho/Planet-Harriers---Recompilation),
blood red: a **launcher** (the four games, online game, settings, about, quit, and a status card)
and a **pause menu** while playing (Esc, F1 or the pad shortcut) with tabs:

* **Game**: resume, reset, TEST switch, screenshot, back to the launcher, quit.
* **Cheats**: see below.
* **Video**: filter (sharp, smooth, scanlines, CRT, curved CRT), scanline strength, picture
  shape (4:3, 16:9, 16:10, fill), integer scaling, fullscreen, volume.
* **Controls**: remap every action to a key (per player) and a pad button (shared by every
  pad, LT/RT triggers included); pad shortcuts for the **menu** (Back+Start, Guide or L3+R3)
  and to **change the filter** while playing (R3, L3, Back+RB or Back+LB). On the Steam Deck
  you can map L4/R4 to L3/R3 in Steam Input.
* **Machine**: DIP switches (free play, blood, violence, low blows, attract sound, country).
* **System**: language, extra ROM folder, search the ROMs again, recompiled games, about.

The menus are drawn at the window's resolution and adapt from 4:3 to 21:9.

## Cheats

Checked in the emulator with the ROMs (addresses from the MAME cheat database, per revision):

| Game | Cheats |
|---|---|
| MK (rev 5.0 / 4.0) | infinite energy P1 / P2, go to the bottom of the Pit |
| MK II (L3.1) | infinite energy P1 / P2, next opponent Smoke, the match before "?" (Jade), the winner fights Noob Saibot |
| MK3 (1.0, 2.0, 2.1) | **unlock Smoke**, infinite energy and run P1 / P2 |
| UMK3 (1.2, 1.1) | **unlock Mileena, Ermac and Classic Sub-Zero**, infinite energy and run P1 / P2 |

The unlocks are stored in the board's CMOS with its checksum fixed (the other settings are
kept) and reset the board. Not available in online games.

## Controls

MK arcade panel: top row **high punch / block / high kick**, bottom row **low punch /
block 2 (MK, MK II) or run (MK3, UMK3) / low kick**. Defaults:

| | Direction | HP | BL | HK | LP | BL2/Run | LK | Start | Coin |
|---|---|---|---|---|---|---|---|---|---|
| P1 | arrows | A | S | D | Z | X | C | 1 | 5 |
| P2 | I J K L | R | T | Y | F | G | H | 2 | 6 |
| Pad | d-pad / stick | Y | RB / RT | B | X | LB / LT | A | Start | Back |

Everything can be changed in **Settings > Controls** (or `mk_recomp.ini`). Esc / Back+Start menu,
F2 TEST switch, F3 reset, F4 filter, F5 integer scaling, F7 aspect, F11 fullscreen, F12 screenshot,
- / + volume, 9 service credit. Launch option `--game mk|mk2|mk3|umk3` opens a game directly
(e.g. from Steam). Everything is stored next to the program (portable: it can live on a USB stick).

## Building

Linux (Ubuntu 24.04: `sudo apt install build-essential libsdl2-dev`):

```sh
make -j build/linux/libcore.a build/linux/miniz.o
make linux-app SDL2L=/usr LINUX_SDL=-lSDL2      # -> build/linux/MKRecomp
```

Windows with [MSYS2](https://www.msys2.org/) (MINGW64 shell,
`pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2`):

```sh
make -j BUILD=build/win CXX=g++ build/win/libcore.a
make win WINCC=gcc WINCXX=g++ SDL2W=/mingw64     # -> dist/MKRecomp/MKRecomp.exe
```

GitHub Actions (`.github/workflows/release.yml`) builds both on every push to `main` (Windows
cross-compiled with MinGW-w64; Linux with a static SDL2 that loads PipeWire / PulseAudio / ALSA and
X11 / Wayland at run time, for SteamOS) and publishes the release of the day (`vYYYY.MM.DD`).

Headless test: `build/linux/cattest <folder with roms/> mk|mk2|mk3|umk3 <frames> <prefix>`
runs a game with scripted inputs and writes `.ppm` snapshots and sound statistics.

## What changed from the NBA JAM base

* **T-Unit / Mortal Kombat**: MK protection (value sequence at `0x1b00000-0x1b6ffff`), the
  sound CPU's hidden RAM (`0xfb9c-0xfbc6`), 256 KB sound program ROM and mirrored 256 KB OKI
  ROMs, 3 graphics banks.
* **T-Unit / Mortal Kombat II**: MK II protection, 1 MB graphics ROMs (`gfx_rom_large`), and a
  **DCS 2K** sound board (2K-word mirrored memory map, latches at `0x3400-0x37ff`) wired to the
  T-Unit at `0x01d00000` / `0x01d01020`.
* **Wolf Unit / MK3 and UMK3**: simulated security PIC (prefix 528), DCS 8K sound, the MK3
  graphics map (512 KB banks in `u114-u117`) and UMK3's (`u110-u113`), plus MAME's tweak for
  UMK3's palette ring buffer (avoids wrong colours).
* **Inputs**: 6 buttons per player with each game's port layout (MK has them in IN1, MK II in
  IN2, MK3 in Wolf Unit port 1).
* **DIP switches**: blood, violence, low blows, attract sound, country and free play.

## License

This project's code is released under the BSD 3-Clause license (see `LICENSE`).
Third-party code: CPU cores and board logic based on MAME (BSD 3-Clause), ymfm (BSD 3-Clause),
miniz (MIT / Unlicense), stb_truetype (public domain / MIT), SDL2 (zlib),
Nosifer, Metal Mania and Oswald fonts (SIL Open Font License, `src/third_party/fonts`).
Mortal Kombat is a trademark of Warner Bros. Entertainment; Midway is a trademark of its
owners. This is a non-profit fan project not affiliated with them.

## Credits

* Midway Games: the original games
* The MAME team (Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles and many more),
  Aaron Giles (ymfm)
* [NBA JAM Recomp](https://github.com/PocketLucho/NBA-JAM---Recompiled): the application base
