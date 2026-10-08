@echo off
rem Registra la extension .rp en Windows: los archivos .rp muestran el logo de RP
rem y se ejecutan con doble clic. No necesita permisos de administrador.
rem Uso (desde la carpeta del proyecto):  instalar\windows\instalar.bat
setlocal
for %%I in ("%~dp0..\..") do set "RAIZ=%%~fI"

if not exist "%RAIZ%\rp.exe" (
    echo No se encontro rp.exe. Primero compile el lenguaje con compilar.bat
    exit /b 1
)

set "CLASES=HKCU\Software\Classes"
reg add "%CLASES%\.rp" /ve /d "RP.Programa" /f >nul
reg add "%CLASES%\.rp" /v "PerceivedType" /d "text" /f >nul
reg add "%CLASES%\RP.Programa" /ve /d "Programa de RP" /f >nul
reg add "%CLASES%\RP.Programa\DefaultIcon" /ve /d "%RAIZ%\logo\rp.ico" /f >nul

rem Doble clic: ejecuta el programa en una consola que queda abierta
reg add "%CLASES%\RP.Programa\shell\open" /ve /d "Ejecutar con RP" /f >nul
reg add "%CLASES%\RP.Programa\shell\open\command" /ve /d "cmd.exe /k \"\"%RAIZ%\rp.exe\" \"%%1\"\"" /f >nul

rem Clic derecho > Editar: abre el codigo en el Bloc de notas
reg add "%CLASES%\RP.Programa\shell\edit" /ve /d "Editar" /f >nul
reg add "%CLASES%\RP.Programa\shell\edit\command" /ve /d "notepad.exe \"%%1\"" /f >nul

rem Pide al Explorador que actualice los iconos
ie4uinit.exe -show >nul 2>nul

echo Listo. Los archivos .rp ya tienen el logo de RP.
echo Doble clic en un .rp: se ejecuta. Clic derecho ^> Editar: abre el codigo.
echo Si el icono no cambia de inmediato, cierre sesion y vuelva a entrar.
