# Mortal Kombat Recomp

*English: [README.md](README.md)*

La saga recreativa **Mortal Kombat** de Midway (MK, MK II, MK3 y UMK3) jugable en una sola
aplicacion de Windows / Linux con menu de juegos, shaders CRT, modos panoramicos y juego en
red. Usa la misma base que [NBA JAM Recomp](https://github.com/PocketLucho/NBA-JAM---Recompiled),
que ya reproduce las dos placas arcade de Midway (T-Unit y Wolf Unit).

Creado por **Pocket_lucho** con **Claude** (Anthropic).
Si quieres apoyarme ve a https://buymeacoffee.com/pocket_lucho

| Juego | Placa | Romset MAME | Estado |
|---|---|---|---|
| Mortal Kombat (1992), rev 5.0 / 4.0 T-Unit | T-Unit + sonido Williams ADPCM (6809, YM2151, OKI6295) | `mk.zip` | jugable, con sonido |
| Mortal Kombat II (1993), rev L3.1 | T-Unit + sonido DCS 2K (ADSP-2105) | `mk2.zip` | jugable, con sonido |
| Mortal Kombat 3 (1995), rev 1.0 / 2.0 / 2.1 | Wolf Unit + DCS 8K | `mk3r10.zip` (o `mk3.zip`) | jugable, con sonido |
| Ultimate Mortal Kombat 3 (1995), rev 1.2 / 1.1 / 1.0 | Wolf Unit + DCS 8K | `umk3.zip` | jugable, con sonido |

Los cuatro se han probado con los romsets de MAME: arrancan, pasan las comprobaciones de
proteccion, muestran el modo demostracion y llegan a los combates con imagen y sonido.

## No incluye datos de los juegos

El codigo, los graficos, el sonido y la musica de Midway tienen derechos de autor, asi que
ni este repositorio ni las versiones compiladas los contienen. Cada usuario copia sus propios
romsets de MAME en `roms/`: `mk.zip`, `mk2.zip`, `mk3r10.zip` (o `mk3.zip`), `umk3.zip`.

La aplicacion recorre `roms/` (subcarpetas y ZIP incluidos), reconoce cada fichero por su
CRC32 (da igual el nombre) y monta lo que necesita cada juego al arrancarlo. Si falta algo,
el juego sale en gris en el menu y al elegirlo se indica que copiar.

## Controles

Panel arcade de MK: fila de arriba **puñetazo alto / bloqueo / patada alta**, fila de abajo
**puñetazo bajo / bloqueo 2 (MK, MK II) o correr (MK3, UMK3) / patada baja**.

| | Direccion | PA | BL | PtA | PB | BL2/Correr | PtB | Start | Moneda |
|---|---|---|---|---|---|---|---|---|---|
| J1 | flechas | A | S | D | Z | X | C | 1 | 5 |
| J2 | I J K L | R | T | Y | F | G | H | 2 | 6 |
| Mando | cruceta / stick | Y | RB / RT | B | X | LB / LT | A | Start | Back |

Esc menu, P pausa, F1 ayuda, F2 interruptor TEST, F3 reset, F4 shader, F5 escalado entero,
F7 aspecto, F11 pantalla completa, F12 captura, 9 credito de servicio. Las teclas y las
opciones de la recreativa (juego libre, pais, sangre, violencia, golpes bajos, sonido en
demo) se cambian en `mk_recomp.ini`.

## Compilar

Linux (Ubuntu 24.04: `sudo apt install build-essential libsdl2-dev`):

```sh
make -j build/linux/libcore.a build/linux/miniz.o
make linux-app SDL2L=/usr LINUX_SDL=-lSDL2      # -> build/linux/MKRecomp
```

Windows con [MSYS2](https://www.msys2.org/) (consola MINGW64,
`pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2`):

```sh
make -j BUILD=build/win CXX=g++ build/win/libcore.a
make win WINCC=gcc WINCXX=g++ SDL2W=/mingw64     # -> dist/MKRecomp/MKRecomp.exe
```

GitHub Actions (`.github/workflows/release.yml`) compila ambas versiones en cada push y las
publica como *release* al crear una etiqueta `v*`.

Prueba sin ventana: `build/linux/cattest <carpeta con roms/> mk|mk2|mk3|umk3 <fotogramas> <prefijo>`
ejecuta un juego con entradas automaticas y guarda capturas `.ppm` y estadisticas de sonido.

## Que se cambio respecto a la base de NBA JAM

* **T-Unit / Mortal Kombat**: proteccion de MK (secuencia de valores en `0x1b00000-0x1b6ffff`),
  RAM oculta de la CPU de sonido (`0xfb9c-0xfbc6`), ROM de programa de sonido de 256 KB y ROMs
  OKI de 256 KB espejadas, 3 bancos de graficos.
* **T-Unit / Mortal Kombat II**: proteccion de MK II, graficos de ROMs de 1 MB (`gfx_rom_large`),
  placa de sonido **DCS 2K** (mapa de memoria de 2K palabras espejado, latches en `0x3400-0x37ff`)
  conectada al T-Unit en `0x01d00000` / `0x01d01020`.
* **Wolf Unit / MK3 y UMK3**: PIC de seguridad simulado (prefijo 528), sonido DCS 8K, mapa de
  graficos de MK3 (bancos de 512 KB en `u114-u117`) y de UMK3 (`u110-u113`), y el mismo ajuste
  que MAME para el bufer circular de paletas de UMK3 (evita colores incorrectos).
* **Entradas**: 6 botones por jugador con la distribucion de puertos de cada juego (MK los
  lleva en IN1, MK II en IN2, MK3 en el puerto 1 del Wolf Unit).
* **Interruptores DIP**: sangre, violencia, golpes bajos, sonido en demo, pais y juego libre.

## Licencia

El codigo de este proyecto se publica con licencia BSD de 3 clausulas (ver `LICENSE`).
Codigo de terceros: nucleos de CPU y logica de las placas basados en MAME (BSD 3 clausulas),
ymfm (BSD 3 clausulas), miniz (MIT / Unlicense), font8x8 (dominio publico), SDL2 (zlib).
Mortal Kombat es una marca de Warner Bros. Entertainment; Midway es marca de sus propietarios.
Es un proyecto de aficionados sin animo de lucro y sin relacion con ellos.

## Creditos

* Midway Games: los juegos originales
* El equipo de MAME (Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles y muchos mas),
  Aaron Giles (ymfm)
* [NBA JAM Recomp](https://github.com/PocketLucho/NBA-JAM---Recompiled): la base de la aplicacion
