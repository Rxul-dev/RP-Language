/* UTILIDADES
 *
 * Funciones de apoyo que usan todos los demás archivos: pedir memoria sin que
 * el programa se caiga si falla, copiar textos, listas que crecen y contar las
 * letras de un texto con tildes (UTF-8).
 */
#include "base/util.h"

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

void *crecer(void *datos, int cantidad, int *capacidad, size_t tam_elemento) {
    if (cantidad < *capacidad) return datos;
    *capacidad = *capacidad ? *capacidad * 2 : 8;
    return xrealloc(datos, (size_t)*capacidad * tam_elemento);
}

void enteros_agregar(ListaEnteros *l, int valor) {
    l->datos = crecer(l->datos, l->cantidad, &l->capacidad, sizeof *l->datos);
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

void agregar_texto(char *buf, size_t n, const char *texto) {
    size_t usado = strlen(buf);
    if (usado + 1 < n) snprintf(buf + usado, n - usado, "%s", texto);
}
