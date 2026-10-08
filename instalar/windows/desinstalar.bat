@echo off
rem Quita el registro de la extension .rp. Uso: instalar\windows\desinstalar.bat
reg delete "HKCU\Software\Classes\.rp" /f >nul 2>nul
reg delete "HKCU\Software\Classes\RP.Programa" /f >nul 2>nul
ie4uinit.exe -show >nul 2>nul
echo Listo: los archivos .rp vuelven a verse como archivos genericos.
