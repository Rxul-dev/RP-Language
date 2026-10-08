@echo off
rem Compila el lenguaje RP en Windows (requiere gcc de MinGW).
rem   compilar.bat          crea rp.exe
rem   compilar.bat prueba   compila y corre las pruebas automaticas

set FUENTES=src\util.c src\valor.c src\errores.c src\token.c src\lexico.c src\ast.c src\sintactico.c src\tabla_simbolos.c src\semantico.c src\interprete.c src\reportes.c src\compilador.c

where gcc >nul 2>nul
if errorlevel 1 (
    echo No se encontro gcc. Instalelo siguiendo el README y vuelva a abrir la terminal.
    exit /b 1
)

if "%1"=="prueba" (
    gcc -std=c11 -Wall -O2 -Isrc -o pruebas\pruebas.exe %FUENTES% pruebas\pruebas.c || exit /b 1
    pruebas\pruebas.exe
    exit /b %errorlevel%
)

gcc -std=c11 -Wall -O2 -o rp.exe %FUENTES% src\main.c || exit /b 1
echo Listo: se creo rp.exe
