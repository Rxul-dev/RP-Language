CC      ?= cc
CFLAGS  ?= -std=c11 -Wall -Wextra -pedantic -O2
FUENTES  = src/util.c src/valor.c src/errores.c src/token.c src/lexico.c src/ast.c \
           src/sintactico.c src/tabla_simbolos.c src/semantico.c src/interprete.c \
           src/reportes.c src/compilador.c
CABECERAS = $(wildcard src/*.h)

rp: $(FUENTES) src/main.c $(CABECERAS)
	$(CC) $(CFLAGS) -o $@ $(FUENTES) src/main.c

pruebas/pruebas: $(FUENTES) pruebas/pruebas.c $(CABECERAS)
	$(CC) $(CFLAGS) -Isrc -o $@ $(FUENTES) pruebas/pruebas.c

prueba: pruebas/pruebas
	./pruebas/pruebas

demo: rp
	./rp ejemplos/completo.rp

clean:
	rm -f rp rp.exe pruebas/pruebas pruebas/pruebas.exe

.PHONY: prueba demo clean
