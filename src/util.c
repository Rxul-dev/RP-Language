#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void sin_memoria(void) {
    fprintf(stderr, "Error fatal: memoria insuficiente\n");
    exit(3);
}

void *xmalloc(size_t n) {
    void *p = malloc(n ? n : 1);
    if (!p) sin_memoria();
    return p;
}

void *xcalloc(size_t cantidad, size_t n) {
    void *p = calloc(cantidad ? cantidad : 1, n ? n : 1);
    if (!p) sin_memoria();
    return p;
}

void *xrealloc(void *p, size_t n) {
    void *nuevo = realloc(p, n ? n : 1);
    if (!nuevo) sin_memoria();
    return nuevo;
}

char *xstrndup(const char *s, size_t n) {
    char *copia = xmalloc(n + 1);
    memcpy(copia, s, n);
    copia[n] = '\0';
    return copia;
}

char *xstrdup(const char *s) {
    return xstrndup(s, strlen(s));
}

void enteros_agregar(ListaEnteros *l, int valor) {
    if (l->cantidad == l->capacidad) {
        l->capacidad = l->capacidad ? l->capacidad * 2 : 4;
        l->datos = xrealloc(l->datos, (size_t)l->capacidad * sizeof *l->datos);
    }
    l->datos[l->cantidad++] = valor;
}

void enteros_liberar(ListaEnteros *l) {
    free(l->datos);
    l->datos = NULL;
    l->cantidad = l->capacidad = 0;
}

size_t utf8_ancho(const char *s) {
    size_t ancho = 0;
    for (; *s; s++) {
        /* Los bytes de continuación UTF-8 tienen la forma 10xxxxxx */
        if (((unsigned char)*s & 0xC0) != 0x80) ancho++;
    }
    return ancho;
}
