CC      ?= cc
CFLAGS  ?= -std=c11 -Wall -Wextra -pedantic -O2
CFLAGS  += -Isrc
# Todo el código de src/ menos main.c (las pruebas tienen su propio main)
FUENTES   = $(filter-out src/main.c, $(wildcard src/*.c src/*/*.c))
CABECERAS = $(wildcard src/*.h src/*/*.h)

rp: $(FUENTES) src/main.c $(CABECERAS)
	$(CC) $(CFLAGS) -o $@ $(FUENTES) src/main.c

pruebas/pruebas: $(FUENTES) pruebas/pruebas.c $(CABECERAS)
	$(CC) $(CFLAGS) -o $@ $(FUENTES) pruebas/pruebas.c

prueba: pruebas/pruebas
	./pruebas/pruebas

demo: rp
	./rp ejemplos/chorizos.rp

clean:
	rm -f rp rp.exe pruebas/pruebas pruebas/pruebas.exe

.PHONY: prueba demo clean
