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
proteccion, muestran el modo demostracion y llegan a los combates con imagen y sonido. Los
cuatro se pueden **recompilar estaticamente** desde tus ROMs (ver abajo).

## Recompilacion estatica (desde tus ROMs)

El codigo fuente arcade de Mortal Kombat nunca se ha publicado, asi que aqui no hay "compilado
desde la fuente" como en algunos juegos de NBA JAM. En su lugar, `tools/build_recomp.sh`
**recompila estaticamente** el programa TMS34010 de cada juego a partir de tus propias ROMs:

1. `tools/recomp/mkrecomp` lee el programa de `roms/`, encuentra el codigo (desde los 32
   vectores de la CPU, los destinos de saltos y llamadas, los punteros a codigo que hay en la
   ROM y las direcciones que el programa anota en `recomp/<juego>.miss`) y escribe C++:
   cada instruccion es una etiqueta; los saltos (JRcc, JAcc, DSJ, DSJS) son condiciones y
   `goto` nativos; las instrucciones de registros usan los mismos macros del interprete con
   el codigo de operacion como constante; el resto llama directamente a su manejador ya
   decodificado.
2. Ese C++ se compila como modulo: `recomp/<juego>.dll` (Windows) o `.so` (Linux / SteamOS). El
   modulo solo usa una interfaz C (punteros al estado de la CPU), asi que vale cualquier
   compilador: el del sistema o Zig, que el script descarga solo si no hay ninguno.
3. Al arrancar el juego, la aplicacion carga el modulo (comprueba que es de esas ROMs y de la
   misma version del programa) y el menu lo marca con **REC**. Lo que no este recompilado
   (codigo en RAM, direcciones no encontradas) lo ejecuta el interprete y queda anotado en
   `recomp/<juego>.miss` para la siguiente generacion.

Tras cada instruccion se comprueban el presupuesto de ciclos y el PC igual que en el bucle
del interprete, asi que el resultado es **identico instruccion a instruccion**: en las pruebas
(1.800-2.400 fotogramas de demo y combate) la imagen y la RAM coinciden en cada fotograma con
el interprete, y tras una pasada de realimentacion el 100 % del codigo ejecutado corre desde
el modulo (MK: ~27.700 instrucciones recompiladas, MK II ~34.400, MK3 ~43.800, UMK3 ~49.100).

**La herramienta viene en la release**, lista para usar y sin instalar nada:

* **Windows**: doble clic en `RECOMPILAR_RECOMPILE.bat` (la primera vez descarga Zig, ~75 MB).
* **Linux / SteamOS**: doble clic en `Recompilar - Recompile.desktop` o `./build_recomp.sh` (usa
  el compilador del sistema; en SteamOS descarga Zig una vez, ~45 MB, en `tools/zig`).

Genera `recomp/<juego>.dll|.so` para cada juego cuyas ROMs esten en `roms/` y borra despues el
C++ generado. Juega un rato y vuelve a ejecutarlo para incluir lo que falto. **Los modulos salen
de ROMs con derechos de autor: no los compartas ni los subas.**

Desde el codigo fuente: `tools/build_recomp.sh <carpeta de MKRecomp> [--windows]`.

## No incluye datos de los juegos

El codigo, los graficos, el sonido y la musica de Midway tienen derechos de autor, asi que
ni este repositorio ni las versiones compiladas los contienen. Cada usuario copia sus propios
romsets de MAME en `roms/`: `mk.zip`, `mk2.zip`, `mk3r10.zip` (o `mk3.zip`), `umk3.zip`.

La aplicacion recorre `roms/` (subcarpetas y ZIP incluidos), reconoce cada fichero por su
CRC32 (da igual el nombre) y monta lo que necesita cada juego al arrancarlo. Si falta algo,
el juego sale en gris en el menu y al elegirlo se indica que copiar.

## Menu

Estilo [Planet Harriers](https://github.com/PocketLucho/Planet-Harriers---Recompilation), en rojo
sangre: **lanzador** (los cuatro juegos, juego en red, ajustes, acerca de, salir, y una tarjeta de
estado) y **menu de pausa** durante la partida (Esc, F1 o el atajo del mando) con pestanas:

* **Partida**: continuar, reiniciar, interruptor TEST, captura, volver al lanzador, salir.
* **Trucos**: ver abajo.
* **Video**: filtro (nitido, suavizado, scanlines, CRT, CRT curvo), intensidad de scanlines, forma
  de la imagen (4:3, 16:9, 16:10, llenar), escalado entero, pantalla completa, volumen.
* **Controles**: reasignar cada accion con una tecla (por jugador) y un boton del mando (comun a
  todos los mandos, con gatillos LT/RT); atajos del mando para el **menu** (Back+Start, Guide o
  L3+R3) y para **cambiar de filtro** jugando (R3, L3, Back+RB o Back+LB). En Steam Deck puedes
  asignar L4/R4 a L3/R3 en Steam Input.
* **Maquina**: interruptores DIP (juego libre, sangre, violencia, golpes bajos, sonido en demo, pais).
* **Sistema**: idioma, carpeta de ROMs extra, volver a buscar ROMs, juegos recompilados, acerca de.

Los menus se dibujan a la resolucion de la ventana y se adaptan de 4:3 a 21:9.

## Trucos

Probados en el emulador con las ROMs (direcciones de la base de trucos de MAME, por revision):

| Juego | Trucos |
|---|---|
| MK (rev 5.0 / 4.0) | energia infinita J1 / J2, ir al fondo del Foso |
| MK II (L3.1) | energia infinita J1 / J2, siguiente rival Smoke, combate antes de "?" (Jade), el ganador lucha contra Noob Saibot |
| MK3 (1.0, 2.0, 2.1) | **desbloquear Smoke**, energia y correr infinitos J1 / J2 |
| UMK3 (1.2, 1.1) | **desbloquear Mileena, Ermac y Sub-Zero clasico**, energia y correr infinitos J1 / J2 |

Los desbloqueos se guardan en la CMOS de la placa con su suma de verificacion corregida (no se
pierden los demas ajustes) y reinician la placa. No estan disponibles en partidas en red.

## Controles

Panel arcade de MK: fila de arriba **punetazo alto / bloqueo / patada alta**, fila de abajo
**punetazo bajo / bloqueo 2 (MK, MK II) o correr (MK3, UMK3) / patada baja**. Por defecto:

| | Direccion | PA | BL | PtA | PB | BL2/Correr | PtB | Start | Moneda |
|---|---|---|---|---|---|---|---|---|---|
| J1 | flechas | A | S | D | Z | X | C | 1 | 5 |
| J2 | I J K L | R | T | Y | F | G | H | 2 | 6 |
| Mando | cruceta / stick | Y | RB / RT | B | X | LB / LT | A | Start | Back |

Todo se cambia en **Ajustes > Controles** (o en `mk_recomp.ini`). Esc / Back+Start menu,
F2 interruptor TEST, F3 reset, F4 filtro, F5 escalado entero, F7 aspecto, F11 pantalla completa,
F12 captura, - / + volumen, 9 credito de servicio. Opcion de lanzamiento `--game mk|mk2|mk3|umk3`
para abrir un juego directamente (por ejemplo desde Steam). Todo se guarda junto al programa
(modo portatil: se puede llevar en un USB).

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

GitHub Actions (`.github/workflows/release.yml`) compila ambas versiones en cada push a `main`
(Windows de forma cruzada con MinGW-w64; Linux con SDL2 estatico que carga PipeWire / PulseAudio
/ ALSA y X11 / Wayland al arrancar, para SteamOS) y publica la *release* del dia (`vAAAA.MM.DD`).

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
ymfm (BSD 3 clausulas), miniz (MIT / Unlicense), stb_truetype (dominio publico / MIT), SDL2 (zlib),
fuentes Nosifer, Metal Mania y Oswald (SIL Open Font License, `src/third_party/fonts`).
Mortal Kombat es una marca de Warner Bros. Entertainment; Midway es marca de sus propietarios.
Es un proyecto de aficionados sin animo de lucro y sin relacion con ellos.

## Creditos

* Midway Games: los juegos originales
* El equipo de MAME (Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles y muchos mas),
  Aaron Giles (ymfm)
* [NBA JAM Recomp](https://github.com/PocketLucho/NBA-JAM---Recompiled): la base de la aplicacion
