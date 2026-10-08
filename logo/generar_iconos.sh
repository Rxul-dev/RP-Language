#!/bin/bash
# Genera rp.icns (Mac) y rp.ico (Windows) a partir de logo.png (1024x1024).
# Se ejecuta en Mac: ./generar_iconos.sh
set -e
cd "$(dirname "$0")"

TMP=$(mktemp -d)
mkdir "$TMP/rp.iconset"
for t in 16 32 128 256 512; do
    sips -z $t $t logo.png --out "$TMP/rp.iconset/icon_${t}x${t}.png" >/dev/null
    sips -z $((t * 2)) $((t * 2)) logo.png --out "$TMP/rp.iconset/icon_${t}x${t}@2x.png" >/dev/null
done
iconutil -c icns "$TMP/rp.iconset" -o rp.icns

for t in 16 32 48 256; do
    sips -z $t $t logo.png --out "$TMP/ico_$t.png" >/dev/null
done
# Un .ico es un índice de imágenes; desde Windows Vista puede contener PNG directamente
python3 - "$TMP" <<'PY'
import struct, sys
carpeta = sys.argv[1]
tamanos = [16, 32, 48, 256]
datos = [open(f"{carpeta}/ico_{t}.png", "rb").read() for t in tamanos]
salida = struct.pack("<HHH", 0, 1, len(tamanos))
desplazamiento = 6 + 16 * len(tamanos)
for t, d in zip(tamanos, datos):
    salida += struct.pack("<BBBBHHII", t % 256, t % 256, 0, 0, 1, 32, len(d), desplazamiento)
    desplazamiento += len(d)
open("rp.ico", "wb").write(salida + b"".join(datos))
PY
rm -rf "$TMP"
echo "Listo: rp.icns y rp.ico"
