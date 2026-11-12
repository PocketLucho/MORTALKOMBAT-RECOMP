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
attract mode and reach the fights with picture and sound.

## No game data included

Midway's code, graphics, sound and music are copyrighted, so neither this repository nor the
builds contain them. Every user copies their own MAME romsets into `roms/`: `mk.zip`,
`mk2.zip`, `mk3r10.zip` (or `mk3.zip`), `umk3.zip`.

The application scans `roms/` (sub-folders and ZIPs included), identifies each file by its
CRC32 (the file name does not matter) and assembles what each game needs when it starts. If
something is missing the game is greyed out in the menu and selecting it says what to copy.

## Controls

MK arcade panel: top row **high punch / block / high kick**, bottom row **low punch /
block 2 (MK, MK II) or run (MK3, UMK3) / low kick**.

| | Direction | HP | BL | HK | LP | BL2/Run | LK | Start | Coin |
|---|---|---|---|---|---|---|---|---|---|
| P1 | arrows | A | S | D | Z | X | C | 1 | 5 |
| P2 | I J K L | R | T | Y | F | G | H | 2 | 6 |
| Pad | d-pad / stick | Y | RB / RT | B | X | LB / LT | A | Start | Back |

Esc menu, P pause, F1 help, F2 TEST switch, F3 reset, F4 shader, F5 integer scaling,
F7 aspect, F11 fullscreen, F12 screenshot, 9 service credit. Keys and the cabinet options
(free play, country, blood, violence, low blows, attract sound) are set in `mk_recomp.ini`.

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

GitHub Actions (`.github/workflows/release.yml`) builds both on every push and publishes them
as a release when a `v*` tag is pushed.

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
miniz (MIT / Unlicense), font8x8 (public domain), SDL2 (zlib).
Mortal Kombat is a trademark of Warner Bros. Entertainment; Midway is a trademark of its
owners. This is a non-profit fan project not affiliated with them.

## Credits

* Midway Games: the original games
* The MAME team (Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles and many more),
  Aaron Giles (ymfm)
* [NBA JAM Recomp](https://github.com/PocketLucho/NBA-JAM---Recompiled): the application base
