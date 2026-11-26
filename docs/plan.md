# Plan del port de Gens4All a NAOMI

Escrito el 2026-11-26 tras leer el código de la 4.3 de TapamN (`gens4all/`). Referencias: la guía de ports de dc2naomi
(`dc2naomi/docs/guia-ports-kallistios.md`) y NesterDC SE (`dc2naomi/homebrew/nesterse/`), el último port hecho así.

## Lo que hemos visto en el código

- **Casi todo pasa por la KallistiOS**: el parche `NAOMI_HOST` (maple, bus G1, FIFO G2, DMA del AICA, PCTRA, cable,
  cima de memoria) debería cubrir lo principal sin tocar el emulador.
- **Vídeo**: `vid_init(DM_640x480)` en el cargador (`src/loader.c:289`) y 320x240 en partida (`src/video/draw.c:150-163`;
  un modo 640x240 propio para 256 de ancho, desactivado con `VDP_STRETCH_256=0`). Dibuja con la API de PVR de KOS y
  además manda vértices al TA por las store queues (`src/pvrvdp/drawrow*.S`, `sq.h`, `QACR0/1`) y toca registros sueltos
  (`PVR_FB_CFG_2`, `src/video/draw.c:181`). `src/main.c:887` incluye `pvr_internal.h` de la KOS: hay que comprobar que
  lo que usa sigue igual en la KOS 680d1862.
- **Sonido**: propio (`src/sounddc/snd_stream.c`), con `snd_init`, `snd_mem_malloc`, `spu_memload`, `spu_dma_transfer`,
  `g2_fifo_wait` y una espera directa al registro de la FIFO G2 (`0xa05f688c`, línea 209). Es la zona que arregla el
  parche (FIFO G2 ×2 y DMA del AICA); la espera directa puede necesitar el mismo arreglo.
- **SDL**: aún se inicia para mando y CD (`src/main.c:764`) y en parte del vídeo (`src/video/draw.c:113`). Quitarlo si
  se puede (menos dependencias de kos-ports); si no, compilar el SDL de kos-ports contra nuestra KOS.
- **Mandos**: `maple_enum_type(…, MAPLE_FUNC_CONTROLLER)` en `src/input.c:373,611`, `src/loader.c:49`, `src/main.c:1097`.
  En la NAOMI el residente traduce JVS a maple: hace falta un mapa de mandos (`pad_map`) para el mueble estándar
  (6 botones + START, 2 jugadores) y una combinación para volver a la lista.
- **Guardado**: SRAM a la VMU con `vmufs` en `/vmu/a1/GENS4ALL_…` (`src/util/save.c:2135-2139`). Con el flag de VMU
  emulada de dc2naomi (`0x8000`) debería valer tal cual, como en NesterDC SE y Mario Kart 64.
- **ROMs y lista**: `GAMELISTFNAME="/cd/gamelist.bin"` (`Makefile.dc`), ROMs por `fs_open` (`src/util/rom.c:618`), en
  `.bin` o `.zip`. En la NAOMI `/cd` lo sirve el residente desde la imagen: el disco reconstruido lleva el emulador, la
  lista y las ROMs (límite de la DIMM: 512 MB, de sobra para cientos de ROMs).
- **Watchdog**: un hilo de KOS (`src/pvrvdp/watchdog.c`) que hace `panic` si el hilo principal no avisa en 4 s. En la
  placa no hay dcload al que volver: desactivarlo o hacer que vuelva a la lista.
- **Núcleo 68K**: FAME para SH4; el ensamblador generado ya viene en `src/cpu/m68k/fame/comp/famegens.s`, así que quizá
  no haga falta GLib. Z80: RAZE (`raze.s.m4` → m4).
- **Compilador**: TapamN usó GCC 9.3 y KOS v2.0.0-489 (sept. 2021). Nosotros, GCC 13-15 y KOS 680d1862: esperar avisos
  convertidos en errores y ensamblador que el `as` nuevo rechace.

## Pasos (cada uno se prueba en Flycast, modo NAOMI, antes de la placa)

1. **Compilar para Dreamcast tal cual** con nuestra KOS sin parche: arreglar lo que no compile con GCC/KOS nuevos.
   Riesgo: `pvr_internal.h`, SDL de kos-ports, ensamblador.
2. **Activar `NAOMI_HOST`** (KOS 680d1862 + `kos-680d1862-naomi_host.patch` de dc2naomi), `build_gens4all.sh` al estilo
   de `build_nesterse.sh`, perfil `gens4all` en `dc2naomi/tools/games.py` (flags como NesterDC SE: `0x9404`).
   Meta: la lista de juegos se ve en Flycast.
3. **Una ROM homebrew libre** en la lista (nada con copyright en el repo) → arranca, se ve y suena en Flycast.
4. **Placa**: arranque, imagen a 31 kHz, sonido, mandos. Caja negra (receta 4) si se queda en negro.
5. **Mandos de recreativa**: mapa nuevo en el residente (o en `src/input.c` bajo `NAOMI_HOST`) y la combinación de salida.
6. **SRAM en la VMU-fichero**, watchdog fuera, 15 kHz para el mueble del Crazy Taxi si interesa.
7. **Lote**: `glc` (en `src/glparse`) genera `gamelist.bin` desde las ROMs del usuario; script que construye el disco.

## Pendiente de decidir

- La rama `megavolt85-hdd` (4.3 con SD/disco duro) no sirve en la NAOMI; mirar si algo suyo (EEPROM SPI) merece la pena.
- `config.mk` falta en el paquete de la 4.3 y su `Makefile.dc` lo incluye: recuperarlo de la v4 en el paso 1.
- Licencia: Gens es GPL; FAME y RAZE tienen licencias propias de uso no comercial. Publicar con los créditos que piden.
