#!/bin/bash
# build_gens4all.sh DISC_SRC [OUT.bin]
#
# Gens4All 4.3 (TapamN, Mega Drive / Genesis emulator for Dreamcast) for the NAOMI / NAOMI 2, rebuilt from this
# repository against KallistiOS 680d1862 + dc2naomi's NAOMI_HOST patch, and packed by dc2naomi as a NAOMI image.
#
#   DISC_SRC  either a folder laid out like the Gens4Dreams disc (gamelist.bin at the top and the ROMs where it
#             says: used as it is), or a folder of ROMs (.md/.bin/.gen/.smd) with optional sub-folders: then
#             tools/mklists.py makes one game list per folder, linked to each other, with titles from the file
#             names (MAIN_TITLE / LINK_TITLE name the links). Its 1ST_READ.BIN and IP.BIN, if any, are NOT used.
#   OUT.bin   default build/gens4all_dc2naomi.bin (+ .gz)
#
# What it does:
#   1. KallistiOS 680d1862 + $D2N/homebrew/mk64/kos-680d1862-naomi_host.patch in its own tree, and zlib (kos-ports).
#   2. SDL[DH] (sdldh/, the SDL that 4.3 links as -lSDLDH) into that KallistiOS.
#   3. Gens4All (gens4all/, make -f Makefile.dc) -> 1ST_READ.BIN (unscrambled).
#   4. Disc: DISC_SRC + our 1ST_READ.BIN + IP.BIN from makeip, then dc2naomi's build.sh with PROFILE.
#
# Needs: sh-elf GCC 13-15 in /opt/toolchains/dc/sh-elf, a dc2naomi clone with the gens4all profiles (D2N, default
# ../dc2naomi), KOS_NAOMI_ENV (environ.sh of a KOS_SUBARCH=naomi tree, for dc2naomi's loader), git, curl, m4, cc,
# python3, genisoimage. Use a clone with LF line ends (WSL, not the Windows one).
# Environment: WORK (default $HOME/g4a-build), D2N, MAIN_TITLE, LINK_TITLE,
#              KOS_NAOMI_ENV (default /opt/toolchains/dc/kos/environ.sh),
#              JOBS (4), PROFILE (gens4all, or gens4all_dbg: KallistiOS log and diagnostics on screen).
set -eo pipefail   # (not -u: KallistiOS's environ.sh reads unset variables)
SRCD=$(realpath "${1:?usage: build_gens4all.sh DISC_SRC [OUT.bin]}")
HERE=$(cd "$(dirname "$0")" && pwd)
D2N=$(realpath "${D2N:-$HERE/../dc2naomi}")
PROFILE=${PROFILE:-gens4all}
OUT=$(realpath -m "${2:-$HERE/build/${PROFILE}_dc2naomi.bin}")
WORK=${WORK:-$HOME/g4a-build}
KOS_NAOMI_ENV=${KOS_NAOMI_ENV:-/opt/toolchains/dc/kos/environ.sh}
JOBS=${JOBS:-4}

KOS_URL=https://github.com/KallistiOS/KallistiOS.git
KOS_REV=680d1862
PORTS_URL=https://github.com/KallistiOS/kos-ports.git
PORTS_REV=f4faacc42faaf552625777b7709e871a827e1055
PATCH=$D2N/homebrew/mk64/kos-680d1862-naomi_host.patch

say() { echo; echo "== $*"; }
[ -f "$PATCH" ] || { echo "no $PATCH: set D2N to a dc2naomi clone" >&2; exit 1; }
grep -q '"gens4all"' "$D2N/tools/games.py" || { echo "$D2N/tools/games.py has no gens4all profile" >&2; exit 1; }
[ -f "$KOS_NAOMI_ENV" ] || { echo "no $KOS_NAOMI_ENV (KallistiOS with KOS_SUBARCH=naomi, for the loader)" >&2; exit 1; }
mkdir -p "$WORK"

# 1. KallistiOS + NAOMI_HOST, own tree; zlib from kos-ports
KOS=$WORK/kos
if [ ! -f "$KOS/lib/dreamcast/libkallisti.a" ] || ! git -C "$KOS" apply -R --check "$PATCH" 2>/dev/null; then
    say "KallistiOS $KOS_REV + NAOMI_HOST"
    [ -d "$KOS/.git" ] || git clone -q "$KOS_URL" "$KOS"
    git -C "$KOS" checkout -q -f "$KOS_REV"
    git -C "$KOS" apply "$PATCH"
    sed -e "s#^export KOS_BASE=.*#export KOS_BASE=\"$KOS\"#" -e "s#^export KOS_PORTS=.*#export KOS_PORTS=\"$WORK/kos-ports\"#" \
        "$KOS/doc/environ.sh.sample" > "$KOS/environ.sh"
    sed -i 's#^export KOS_CFLAGS="${KOS_CFLAGS} -O2"#export KOS_CFLAGS="${KOS_CFLAGS} -DNAOMI_HOST"\n&#' "$KOS/environ.sh"
    grep -q NAOMI_HOST "$KOS/environ.sh"
    rm -f "$KOS/lib/dreamcast/libkallisti.a"
    (source "$KOS/environ.sh" && cd "$KOS" && make -j"$JOBS")
fi
source "$KOS/environ.sh" >/dev/null 2>&1
[ -d "$WORK/kos-ports/.git" ] || git clone -q "$PORTS_URL" "$WORK/kos-ports"
git -C "$WORK/kos-ports" checkout -q "$PORTS_REV"
if [ ! -f "$KOS_PORTS/lib/libz.a" ]; then
    # zlib.net keeps only the latest zlib: 1.3.2 from its GitHub releases (kos-ports checks size and SHA-256)
    Z=$WORK/kos-ports/zlib/dist/zlib-1.3.2.tar.gz
    [ "$(stat -c %s "$Z" 2>/dev/null)" = 1502830 ] ||
        { mkdir -p "$(dirname "$Z")"; curl -sfL -o "$Z" https://github.com/madler/zlib/releases/download/v1.3.2/zlib-1.3.2.tar.gz; }
    say "kos-ports: zlib"; (cd "$KOS_PORTS/zlib" && make install clean)
fi

# GCC 13-15 make errors of what GCC 9.3 only warned about (as in gens4all/Makefile.dc)
OLDC="-std=gnu17 -Wno-error=implicit-function-declaration -Wno-error=implicit-int -Wno-error=incompatible-pointer-types
      -Wno-error=int-conversion -Wno-error=return-mismatch"

# 2. SDL[DH] (fresh copy every time)
say "SDL[DH]"
rm -rf "$WORK/sdldh"; cp -r "$HERE/sdldh" "$WORK/sdldh"
(export KOS_CFLAGS="$KOS_CFLAGS $(echo $OLDC) -w"; make -C "$WORK/sdldh" -j"$JOBS" >/dev/null)   # env, not argument: its += must apply
mkdir -p "$KOS_BASE/addons/include/SDL"; cp "$WORK"/sdldh/include/*.h "$KOS_BASE/addons/include/SDL/"
[ -f "$KOS_BASE/addons/lib/dreamcast/libSDLDH.a" ]

# 3. Gens4All (fresh copy every time; libsh4/ is found through -Isrc/cpu/m68k + "../../../libsh4")
say "Gens4All"
SRC=$WORK/gens4all; rm -rf "$SRC"; cp -r "$HERE/gens4all" "$SRC"
make -C "$SRC" -f Makefile.dc -j"$JOBS" > "$WORK/gens4all.log" 2>&1 || { tail -30 "$WORK/gens4all.log"; exit 1; }
[ -f "$SRC/gens4all.bin" ]

# 4. disc and dc2naomi
DISC=$WORK/disc; rm -rf "$DISC"; mkdir -p "$DISC"
make -C "$SRC/src/glparse" >/dev/null
if [ -f "$SRCD/gamelist.bin" ]; then
    (cd "$SRCD" && find . -mindepth 1 -maxdepth 1 ! -iname 1ST_READ.BIN ! -iname IP.BIN -exec cp -r {} "$DISC/" \;)
else
    say "game lists (tools/mklists.py)"
    python3 -I "$HERE/tools/mklists.py" "$SRC/src/glparse/glc" "$SRCD" "$DISC" \
        --main-title "${MAIN_TITLE:-[[ Main list ]]}" --link-title "${LINK_TITLE:-[[ %s ]]}"
fi
cp "$SRC/gens4all.bin" "$DISC/1ST_READ.BIN"
printf '%s\n' "Device Info   : CD-ROM1/1" "Area Symbols  : JUE" "Peripherals   : E000F10" "Product No    : T0000" \
    "Version       : V1.000" "Release Date  : 20261126" "Boot Filename : 1ST_READ.BIN" "SW Maker Name : GENS4ALL NAOMI" \
    "Game Title    : GENS4ALL" > "$WORK/ip.txt"
"$KOS/utils/makeip/makeip" "$WORK/ip.txt" "$DISC/IP.BIN" >/dev/null
say "dc2naomi: $PROFILE -> $OUT"
cd "$D2N"; mkdir -p "$(dirname "$OUT")"
(source "$KOS_NAOMI_ENV" && REBUILD=1 ./build.sh "$PROFILE" "$DISC" "$OUT")
python3 -I - "$OUT" <<'PY'
import gzip, os, sys
src = sys.argv[1]; name = os.path.basename(src)
with open(src + ".gz", "wb") as raw, gzip.GzipFile(filename=name, mode="wb", fileobj=raw, compresslevel=6, mtime=0) as gz:
    with open(src, "rb") as f:
        while (b := f.read(1 << 20)): gz.write(b)
print(src + ".gz", os.path.getsize(src + ".gz"), "bytes")
PY
