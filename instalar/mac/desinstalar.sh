#!/bin/bash
# Quita la app "RP" y el registro de la extensión .rp. Uso: ./instalar/mac/desinstalar.sh
APP="$HOME/Applications/RP.app"
LSREGISTER=/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister
[ -d "$APP" ] && "$LSREGISTER" -u "$APP"
rm -rf "$APP"
killall Finder 2>/dev/null || true
echo "Listo: se quitó la app RP. Los archivos .rp vuelven a verse como archivos genéricos."
