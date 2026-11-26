# gens4all-naomi

**Gens4All** (el emulador de Mega Drive / Genesis para Dreamcast que mejoró TapamN entre 2021 y 2022) en la
**NAOMI / NAOMI 2** por netboot. Es el mismo método que usamos con NesterDC SE y Mario Kart 64 en
[dc2naomi](https://github.com/0skarzer0/dc2naomi): el emulador se compila contra una KallistiOS parcheada para NAOMI
(`NAOMI_HOST`) y dc2naomi lo empaqueta, con las ROMs y la lista de juegos, como imagen de NAOMI.

**Estado:** compila para NAOMI (`build_gens4all.sh`) y funciona en la placa (NAOMI 2, mueble del Crazy Taxi): lista de
juegos, sonido y mandos probados; los fondos, arreglados después y aún sin reprobar en placa. Ver [docs/plan.md](docs/plan.md).

## Qué hay aquí

- `gens4all/`: el código de Gens4All **4.3 de TapamN** (2022-01-24), el mismo que lleva la recopilación Gens4Dreams
  RC3, tal como lo publicó y sin sus binarios. La historia de `main` tiene una versión por commit (de agosto de 2021 a
  la 4.3); la rama `megavolt85-hdd` guarda su 4.3 con SD y disco duro. Los cambios para NAOMI van en commits aparte,
  bajo `#ifdef NAOMI_HOST`, para que se vea qué es suyo y qué es nuestro.
- `docs/plan.md`: lo que hay que cambiar y en qué orden.

En este repositorio no hay nada con copyright de terceros fuera del propio emulador: ni ROMs, ni discos, BIOS o
imágenes. Las ROMs las pone cada uno.

## Versiones de Gens4All

| Versión | Fecha | Código | En este repo |
|---|---|---|---|
| Z80 (sin número) | 2021-08-18 | completo, sin cargador | `main` |
| "with loader" | 2021-08-23 | completo | `main` |
| (menú en partida) | 2021-09-20 | completo | `main` |
| v4 (rasters, lista de juegos) | 2021-11-24 | completo | `main` |
| 4.1 / 4.2 | 2021-12 / 2022-01 | la 4.2 solo en binario (`1ST_READ.BIN`) | no |
| **4.3** | 2022-01-24 | **completo: base del port** | `main` |
| 4.3 con SD y disco duro (megavolt85) | 2022-11-25 | completo | rama `megavolt85-hdd` |

Todas salen del hilo "Gens4All with Z80 Emulation" de Dreamcast-Talk. La 4.3 se ha comprobado: su `1ST_READ.BIN` es
idéntico al de Gens4Dreams RC3.

## Agradecimientos

- **TapamN**: Gens4All con Z80, vídeo por PVR, rasters, lista de juegos y menú en partida.
- **Chui** y **Daniel Lancha**: los primeros ports de Gens a Dreamcast, de los que sale Gens4All.
- **Stéphane Dallongeville**: Gens.
- **FAME Motorola 68000 Emulation Library by Oscar Orallo** (oscar@m68k.com), uso no comercial (ver
  `gens4all/src/cpu/m68k/fame/docs/fame.html`).
- **RAZE** (Z80, Richard Mitton), con su propia licencia.
- **megavolt85**: la 4.3 con disco duro. **WedgeStratos**: la recopilación Gens4Dreams.
- [KallistiOS](https://github.com/KallistiOS/KallistiOS).
