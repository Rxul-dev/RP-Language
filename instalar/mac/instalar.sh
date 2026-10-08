#!/bin/bash
# Instala la app "RP" en ~/Applications para que los archivos .rp tengan el logo
# del lenguaje y se ejecuten con doble clic. Uso: ./instalar/mac/instalar.sh
set -e
AQUI="$(cd "$(dirname "$0")" && pwd)"
RAIZ="$(cd "$AQUI/../.." && pwd)"
APP="$HOME/Applications/RP.app"
LSREGISTER=/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister

echo "1/4 Compilando el lenguaje..."
make -s -C "$RAIZ"

echo "2/4 Creando la app RP..."
mkdir -p "$HOME/Applications"
rm -rf "$APP"
osacompile -o "$APP" "$AQUI/RP.applescript" 2>/dev/null
cp "$RAIZ/rp" "$APP/Contents/Resources/rp"
cp "$RAIZ/logo/rp.icns" "$APP/Contents/Resources/applet.icns"
cp "$RAIZ/logo/rp.icns" "$APP/Contents/Resources/rp.icns"
rm -f "$APP/Contents/Resources/Assets.car"

echo "3/4 Registrando la extensión .rp..."
# Declara el tipo de archivo "Programa de RP" (.rp) con su icono, y la app como la que lo abre
PLIST="$APP/Contents/Info.plist"
pb() { /usr/libexec/PlistBuddy -c "$1" "$PLIST"; }
pb "Delete :CFBundleIconName" 2>/dev/null || true
pb "Delete :CFBundleDocumentTypes" 2>/dev/null || true
pb "Delete :UTExportedTypeDeclarations" 2>/dev/null || true
poner() { pb "Set :$1 $2" 2>/dev/null || pb "Add :$1 string $2"; }
poner CFBundleIdentifier gt.lenguaje.rp
poner CFBundleName RP
poner CFBundleIconFile applet

pb "Add :UTExportedTypeDeclarations array"
pb "Add :UTExportedTypeDeclarations:0 dict"
pb "Add :UTExportedTypeDeclarations:0:UTTypeIdentifier string gt.lenguaje.rp.codigo"
pb "Add :UTExportedTypeDeclarations:0:UTTypeDescription string Programa de RP"
pb "Add :UTExportedTypeDeclarations:0:UTTypeIconFile string rp.icns"
pb "Add :UTExportedTypeDeclarations:0:UTTypeConformsTo array"
pb "Add :UTExportedTypeDeclarations:0:UTTypeConformsTo:0 string public.plain-text"
pb "Add :UTExportedTypeDeclarations:0:UTTypeTagSpecification dict"
pb "Add :UTExportedTypeDeclarations:0:UTTypeTagSpecification:public.filename-extension array"
pb "Add :UTExportedTypeDeclarations:0:UTTypeTagSpecification:public.filename-extension:0 string rp"

pb "Add :CFBundleDocumentTypes array"
pb "Add :CFBundleDocumentTypes:0 dict"
pb "Add :CFBundleDocumentTypes:0:CFBundleTypeName string Programa de RP"
pb "Add :CFBundleDocumentTypes:0:CFBundleTypeRole string Viewer"
pb "Add :CFBundleDocumentTypes:0:CFBundleTypeIconFile string rp.icns"
pb "Add :CFBundleDocumentTypes:0:LSHandlerRank string Owner"
pb "Add :CFBundleDocumentTypes:0:LSItemContentTypes array"
pb "Add :CFBundleDocumentTypes:0:LSItemContentTypes:0 string gt.lenguaje.rp.codigo"
codesign --force --deep -s - "$APP" 2>/dev/null
"$LSREGISTER" -f "$APP"
# macOS solo muestra el logo de apps que se han abierto al menos una vez
open -g -j -W "$APP"

echo "4/4 Actualizando los iconos del Finder..."
touch "$RAIZ"/ejemplos/*.rp
killall Finder 2>/dev/null || true

echo
echo "Listo. Los archivos .rp ya tienen el logo de RP."
echo "Doble clic en un .rp: se ejecuta en la Terminal."
echo "Para editarlo: clic derecho > Abrir con > VS Code o TextEdit."
