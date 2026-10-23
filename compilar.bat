@echo off
rem Compila el lenguaje RP en Windows (requiere gcc de MinGW).
rem   compilar.bat          crea rp.exe
rem   compilar.bat prueba   compila y corre las pruebas automaticas

set FUENTES=src\compilador.c src\interactivo.c src\reportes.c ^
 src\base\util.c src\base\valor.c src\base\errores.c ^
 src\fases\token.c src\fases\lexico.c src\fases\ast.c src\fases\sintactico.c ^
 src\fases\tabla_simbolos.c src\fases\semantico.c src\fases\interprete.c ^
 src\instrucciones\expresiones.c src\instrucciones\declaracion.c src\instrucciones\asignacion.c ^
 src\instrucciones\consulta.c src\instrucciones\contame.c src\instrucciones\si.c ^
 src\instrucciones\chambas.c src\instrucciones\chorizos.c

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

gcc -std=c11 -Wall -O2 -Isrc -o rp.exe %FUENTES% src\main.c || exit /b 1
echo Listo: se creo rp.exe
