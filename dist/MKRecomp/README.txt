Mortal Kombat Recomp - Midway's Mortal Kombat arcade saga
=========================================================
Created by Pocket_lucho with Claude (Anthropic).
Project: https://github.com/PocketLucho/MORTALKOMBAT-RECOMP
If you want to support me, go to https://buymeacoffee.com/pocket_lucho

Windows: run MKRecomp.exe (64-bit Windows 10/11, no installation).
Linux / SteamOS: run MKRecomp (see "LINUX AND STEAMOS" below).
A menu lets you pick the game (arrows + Enter, or the pad). Esc returns to the
menu; in the menu, Esc / B goes to "Quit" and a second press closes the program.
The language (castellano / English) is changed in the menu. The menus adapt to
widescreen displays (16:9, 16:10, 21:9).

(Castellano: ver LEEME.txt)


IMPORTANT: YOU SUPPLY THE GAME DATA
-----------------------------------
The program does NOT include any Midway graphics, sound, music or code: they
are copyrighted and cannot be distributed. Copy your own MAME romsets into the
roms\ folder. The program finds them by itself (by CRC32, whatever the file
name, inside .zip files too); there is no need to unpack anything.

  Game                           What to copy into roms\
  -----------------------------  ------------------------------------------
  Mortal Kombat (1992)           mk.zip      (rev 5.0 or 4.0 T-Unit)
  Mortal Kombat II (1993)        mk2.zip     (rev L3.1)
  Mortal Kombat 3 (1995)         mk3r10.zip  (rev 1.0) or mk3.zip (rev 2.1)
  Ultimate Mortal Kombat 3       umk3.zip    (rev 1.2, 1.1 or 1.0)

A game with missing files is greyed out; selecting it tells you what to copy.


STATIC RECOMPILATION (optional, recommended)
--------------------------------------------
  Turns each game's TMS34010 program into native C++ code built from YOUR ROMs.
  Nothing to install:
    Windows:          double-click RECOMPILAR_RECOMPILE.bat
    Linux / SteamOS:  double-click "Recompilar - Recompile" or ./build_recomp.sh
  The first time, without a compiler (Windows, SteamOS), it downloads Zig into
  tools/zig (~45-75 MB). It builds recomp/<game>.dll / .so; the launcher marks
  those games with REC. Play for a while and run it again: the code that was
  missed (recomp/*.miss) is added. Do not share those files: they come from
  your ROMs.
  Steam: the launch option  --game mk  (or mk2, mk3, umk3) opens that game
  directly.


LINUX AND STEAMOS (Steam Deck, Legion Go S...)
----------------------------------------------
  Nothing to install: the program carries what it needs and uses the system
  sound (PipeWire/PulseAudio/ALSA). Needs x86-64 Linux with glibc 2.35 or
  newer (SteamOS, Ubuntu 22.04+, Fedora 36+...).

  1. In desktop mode unpack the .tar.gz into a folder of yours, for example
     /home/deck/Games/MKRecomp (Dolphin: right click -> Extract -> Extract
     here). Do not run it from inside the .tar.gz.
  2. Copy your romsets into the roms/ folder.
  3. Double-click MKRecomp. If it does not start: right click -> Properties ->
     Permissions -> tick "Is executable".
  4. To play in game mode: in Steam (desktop mode) "Games -> Add a Non-Steam
     Game", click "Browse", pick MKRecomp and accept.
     On the pad: Back + Start returns to the game menu (Steam takes the
     Steam/Guide button).
  If something goes wrong, look at mk_recomp.log next to the program.


CONTROLS AND MENU
-----------------
Top row:    high punch / block / high kick
Bottom row: low punch / block 2 (MK, MK II) or run (MK3) / low kick

  P1: arrows, A S D (top), Z X C (bottom), start 1, coin 5
  P2: I J K L, R T Y (top), F G H (bottom), start 2, coin 6
  Pad: Y high punch, X low punch, B high kick, A low kick,
       RB/RT block, LB/LT block 2 / run, Start, Back = coin

  While playing, Esc or Back + Start open the MENU: resume, reset, CHEATS
  (infinite energy; unlock Smoke in MK3 and Mileena, Ermac and Classic
  Sub-Zero in UMK3...), VIDEO (CRT / scanline filters, picture shape, volume),
  CONTROLS (change every key and pad button, and the shortcuts), MACHINE
  (blood, violence, free play...) and SYSTEM (language, extra ROM folder).
  R3 (right stick click) changes the picture filter while playing; on the
  Steam Deck you can map L4/R4 to R3 in Steam Input.

  F2 TEST  F3 reset  F4 filter  F5 integer scaling  F7 aspect
  F11 fullscreen  F12 screenshot  -/+ volume  9 service credit

Everything is stored next to the program (mk_recomp.ini, saves/): it can live
on a USB stick. Each game's scores and settings are
saved in saves\.
